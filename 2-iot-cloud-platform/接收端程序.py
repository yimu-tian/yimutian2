import logging
import json
import base64
from PIL import Image
import pygame  # 用于播放音频
from linkkit import linkkit
import time  # 确保导入time模块

# 设置日志记录
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

# 初始化LinkKit连接
lk = linkkit.LinkKit(
    host_name="cn-shanghai",
    product_key="YOUR_PRODUCT_KEY",  # 替换为对应设备的ProductKey
    device_name="Receiver",  # 替换为对应设备的DeviceName
    device_secret="YOUR_DEVICE_SECRET",  # 替换为对应设备的DeviceSecret
)

lk.enable_logger(logging.DEBUG)
lk.config_mqtt(endpoint="YOUR_PRODUCT_KEY.iot-as-mqtt.cn-shanghai.aliyuncs.com")
lk.connect_async()

# 订阅主题
def subscribe_topic():
    try:
        lk.subscribe_topic("/sys/YOUR_PRODUCT_KEY/Receiver/user/get", qos=0)  # 使用QoS 0进行订阅
        logger.debug("Subscription successful")
    except Exception as e:
        logger.error(f"Subscription failed: {e}")

# 调用订阅函数
subscribe_topic()

# 设置接收消息的回调函数
def on_topic_message(topic, payload, qos, userdata):
    """
    处理接收到的消息
    """
    logger.debug(f"Received message on topic: {topic}")

    try:
        dd = json.loads(payload)

        if "image" in dd:  # 如果消息包含图像
            logger.debug("Processing image data")
            # 解码图片数据
            image_data = base64.b64decode(dd["image"])

            # 保存图像文件
            with open("received_image.png", "wb") as f:
                f.write(image_data)
            logger.debug("Image saved as received_image.png")

            # 打开并显示图片
            image = Image.open("received_image.png")
            image.show()

        elif "audio" in dd:  # 如果消息包含音频
            logger.debug("Processing audio data")
            try:
                # 打印接收到的音频数据（仅打印前50个字符，避免日志过长）
                logger.debug(f"Raw audio data: {dd['audio'][:50]}...")

                # 解码音频数据
                audio_data = base64.b64decode(dd["audio"])

                # 打印解码后的数据长度
                logger.debug(f"Decoded audio data length: {len(audio_data)} bytes")

                # 保存音频文件
                with open("received_audio.wav", "wb") as f:
                    f.write(audio_data)
                logger.debug("Audio saved as received_audio.wav")

                # 使用pygame播放音频
                pygame.mixer.init()
                pygame.mixer.music.load("received_audio.wav")
                pygame.mixer.music.play()
                logger.debug("Audio playback started")

            except Exception as e:
                logger.error(f"Error processing audio: {e}")

        else:
            logger.warning("Received message does not contain image or audio")

    except Exception as e:
        logger.error(f"Error occurred while processing the message: {e}")

lk.on_topic_message = on_topic_message

# 持续接收消息
while True:
    time.sleep(1)  # 使用time.sleep来避免CPU过度占用