import logging
from linkkit import linkkit
import tkinter as tk
from tkinter import messagebox, filedialog
import json
import base64
import time
import random
import matplotlib.pyplot as plt
import numpy as np
from PIL import Image, ImageTk

# 配置LinkKit连接
lk = linkkit.LinkKit(
    host_name="cn-shanghai",
    product_key="YOUR_PRODUCT_KEY",  # 替换为对应设备的ProductKey
    device_name="Sender",  # 替换为对应设备的DeviceName
    device_secret="YOUR_DEVICE_SECRET",  # 替换为对应设备的DeviceSecret
)

lk.enable_logger(logging.DEBUG)
lk.config_mqtt(
    endpoint="YOUR_PRODUCT_KEY.iot-as-mqtt.cn-shanghai.aliyuncs.com"  # 按照以下格式填写:{product_key}.iot-as-mqtt.{region_id}.aliyuncs.com
)
lk.config_mqtt(
    port=8883,
    protocol="MQTTv311",
    transport="TCP",
    secure="TLS",
    keep_alive=60,
    clean_session=True,
    max_inflight_message=20,
    max_queued_message=0,
    auto_reconnect_min_sec=1,
    auto_reconnect_max_sec=60,
    cadata=None,
)

# 设备动态注册回调函数
def on_device_dynamic_register(rc, value, userdata):
    if rc == 0:
        global device_secret
        print("dynamic register device success, rc:%d, value:%s" % (rc, value))
        device_secret = value
    else:
        print("dynamic register device fail, rc:%d, value:%s" % (rc, value))

lk.on_device_dynamic_register = on_device_dynamic_register
lk.connect_async()

# 等待设备连接成功
time.sleep(5)

# 创建Tkinter窗口
window = tk.Tk()
window.title("数据发送")
window.geometry("600x400")

# 加载背景图片
background_image = Image.open(r"D:\music\abab.png")
background_photo = ImageTk.PhotoImage(background_image)

# 创建一个Label作为背景
background_label = tk.Label(window, image=background_photo)
background_label.place(x=0, y=0, relwidth=1, relheight=1)

# 创建一个Frame作为容器
frame = tk.Frame(window, padx=20, pady=20)
frame.place(relx=0.5, rely=0.5, anchor="center")

# 创建一个Label显示标题
title_label = tk.Label(frame, text="数据发送", font=("Arial", 20, "bold"))
title_label.pack(pady=20)

def send_audio(audio_data):
    # 将音频数据进行Base64编码
    audio_base64 = base64.b64encode(audio_data).decode("utf-8")
    # 将音频数据转换为JSON格式
    audio_payload = json.dumps({"audio": audio_base64})
    # 发送音频消息到阿里云物联网平台
    lk.publish_topic("/sys/YOUR_PRODUCT_KEY/Sender/user/update", audio_payload)  # 修改为系统topic
    # 提示框显示发送结果
    messagebox.showinfo("发送音频", "已发送音频消息")

def choose_audio_file():
    # 弹出文件对话框，选择音频文件
    file_path = filedialog.askopenfilename(title="选择音频文件", filetypes=[("音频文件", "*.mp3;*.wav;*.flac")])
    if file_path:
        with open(file_path, "rb") as audio_file:
            audio_data = audio_file.read()
            send_audio(audio_data)

def generate_plot(data_type):
    # 根据不同的分布类型生成数据
    plot_filename = f"{data_type}_plot.png"
    try:
        if data_type == "line":
            x = range(10)
            y = [random.randint(1, 100) for _ in x]
            plt.plot(x, y)
        elif data_type == "normal":
            mu = 0
            sigma = 1
            data = np.random.normal(mu, sigma, 500)
            plt.hist(data, bins=10)
        elif data_type == "poisson":
            lambda_ = 10
            data = np.random.poisson(lambda_, 100)
            plt.hist(data, bins=10)
        plt.savefig(plot_filename)
        plt.clf()
        with open(plot_filename, "rb") as file:
            image_data = file.read()
        send_image(image_data)
        messagebox.showinfo(f"发送{data_type}图", f"已发送{data_type}图像")
    except Exception as e:
        messagebox.showerror("图像生成失败", f"生成{data_type}图像时出错: {str(e)}")

def send_image(image_data):
    # 将图像数据进行Base64编码
    image_base64 = base64.b64encode(image_data).decode("utf-8")
    # 将图像数据转换为JSON格式
    image_payload = json.dumps({"image": image_base64})
    # 发送图像消息到阿里云物联网平台
    lk.publish_topic("/sys/YOUR_PRODUCT_KEY/Sender/user/update", image_payload)  # 修改为系统topic

# 创建发送折线图按钮
line_button = tk.Button(frame, text="发送折线图", command=lambda: generate_plot("line"), width=20, height=2)
line_button.pack(side="left", padx=10, pady=10)

# 创建发送正态分布图按钮
normal_button = tk.Button(frame, text="发送正态分布图", command=lambda: generate_plot("normal"), width=20, height=2)
normal_button.pack(side="left", padx=10, pady=10)

# 创建发送泊松分布图按钮
poisson_button = tk.Button(frame, text="发送泊松分布图", command=lambda: generate_plot("poisson"), width=20, height=2)
poisson_button.pack(side="left", padx=10, pady=10)

# 创建发送音频文件按钮
audio_button = tk.Button(frame, text="发送音频", command=choose_audio_file, width=20, height=2)
audio_button.pack(pady=10)

# 运行Tkinter窗口
window.mainloop()