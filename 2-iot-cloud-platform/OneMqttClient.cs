using System;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Protocol;
using Newtonsoft.Json;

namespace IOTAPP_Simulator   // 监控项目请将命名空间改为 IOTCTRL_Monitor
{
    public class OneMqttClient
    {
        private IMqttClient? _mqttClient;
        private readonly string _productId;
        private readonly string _deviceName;
        private readonly string _accessKey;

        public bool IsConnected => _mqttClient?.IsConnected ?? false;

        public event Func<string, Task>? OnMessageReceived;

        public OneMqttClient(string productId, string deviceName, string accessKey)
        {
            _productId = productId;
            _deviceName = deviceName;
            _accessKey = accessKey;
        }

        private string GenerateToken(int expirySeconds = 31536000)
        {
            string res = $"products/{_productId}/devices/{_deviceName}";
            long et = DateTimeOffset.UtcNow.ToUnixTimeSeconds() + expirySeconds;
            string strToSign = $"et={et}&method=sha1&res={res}";
            byte[] keyBytes = Convert.FromBase64String(_accessKey);
            using var hmac = new HMACSHA1(keyBytes);
            byte[] signBytes = hmac.ComputeHash(Encoding.UTF8.GetBytes(strToSign));
            string sign = Convert.ToBase64String(signBytes);
            string encodedRes = Uri.EscapeDataString(res);
            return $"version=2018-10-31&res={encodedRes}&et={et}&method=sha1&sign={Uri.EscapeDataString(sign)}";
        }

        public async Task ConnectAsync()
        {

            string fixedToken = "YOUR_DEVICE_TOKEN";


            var options = new MqttClientOptionsBuilder()
                .WithTcpServer("mqtts.heclouds.com", 1883)
                .WithClientId(_deviceName)
                .WithCredentials(_productId, fixedToken)
                .WithCleanSession()
                .WithKeepAlivePeriod(TimeSpan.FromSeconds(120))
                .Build();

            var factory = new MqttFactory();
            _mqttClient = factory.CreateMqttClient();
            _mqttClient.ApplicationMessageReceivedAsync += OnMessageReceivedAsync;
            await _mqttClient.ConnectAsync(options, CancellationToken.None);
        }

        public async Task DisconnectAsync()
        {
            if (_mqttClient != null)
            {
                await _mqttClient.DisconnectAsync();
                _mqttClient.Dispose();
                _mqttClient = null;
            }
        }

        public async Task PublishAsync(string topic, object payload)
        {
            if (!IsConnected)
                throw new InvalidOperationException("MQTT 客户端未连接");

            string json = JsonConvert.SerializeObject(payload);
            var message = new MqttApplicationMessageBuilder()
                .WithTopic(topic)
                .WithPayload(Encoding.UTF8.GetBytes(json))
                .WithQualityOfServiceLevel(MqttQualityOfServiceLevel.AtLeastOnce)
                .Build();
            await _mqttClient!.PublishAsync(message, CancellationToken.None);
        }

        public async Task SubscribeAsync(string topic)
        {
            if (!IsConnected)
                throw new InvalidOperationException("MQTT 客户端未连接");

            var filter = new MqttTopicFilterBuilder()
                .WithTopic(topic)
                .WithQualityOfServiceLevel(MqttQualityOfServiceLevel.AtLeastOnce)
                .Build();
            await _mqttClient!.SubscribeAsync(filter, CancellationToken.None);
        }

        private Task OnMessageReceivedAsync(MqttApplicationMessageReceivedEventArgs e)
        {
            if (OnMessageReceived != null)
            {
                // 注意：5.x 中 PayloadSegment 是 ReadOnlyMemory<byte>，直接使用 ToArray()
                byte[] payloadBytes = e.ApplicationMessage.PayloadSegment.ToArray();
                string payload = Encoding.UTF8.GetString(payloadBytes);
                return OnMessageReceived.Invoke(payload);
            }
            return Task.CompletedTask;
        }
    }
}