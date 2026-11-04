# 物联网云平台（MQTT）

基于 MQTT 协议的物联网云平台接入项目，对接阿里云物联网平台与 OneNET 两大主流云平台。

## 项目组成

| 子项目 | 云平台 | 技术 | 功能 |
|--------|--------|------|------|
| aliyun-linkkit | 阿里云物联网平台 | Python + linkkit SDK | 发布端/接收端，图片、音频数据传输 |
| onenet-mqttnet | OneNET（中国移动） | C# + MQTTnet | WinForms 监控上位机，远程控制 |

## 技术要点

- MQTT 发布/订阅模型，QoS 分级
- 设备三元组认证（ProductKey + DeviceName + DeviceSecret）
- HMACSHA1 / MD5 签名认证
- 设备数据上报与远程控制指令下发

## 目录结构

```
├── aliyun-linkkit/    # 阿里云（Python）
└── onenet-mqttnet/    # OneNET（C#）
```

## 说明

代码中的云平台密钥已脱敏，替换为你自己的三元组即可运行。
