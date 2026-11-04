using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Protocol;
using Newtonsoft.Json.Linq;

namespace IOTCTRL_Monitor
{
    public partial class Form1 : Form
    {
        // ==================== 请修改为你的真实信息 ====================
        private const string PRODUCT_ID = "YOUR_PRODUCT_ID";
        private const string DEVICE_NAME = "IOTAPP_Simulator";   // 使用仿真设备凭证
        private const string DEVICE_TOKEN = "YOUR_DEVICE_TOKEN";
        // ============================================================

        private IMqttClient? _mqttClient;
        private System.Windows.Forms.Timer _timerReport;   // 定时上报数据
        private Random _rand = new Random();

        // 模拟数据（同时也是实际显示的数据）
        private double _temperature = 25.0;
        private double _ph = 7.0;
        private double _dissolvedOxygen = 5.0;
        private bool _heatState = false;
        private bool _coolState = false;
        private bool _acidState = false;
        private bool _alkaliState = false;
        private bool _stirState = false;
        private int _controlMode = 0;   // 0=本地,1=远程

        // UI 控件
        private Label? lblTempVal, lblPhVal, lblDoVal;
        private Label? lblHeatState, lblCoolState, lblAcidState, lblAlkaliState, lblStirState, lblControlMode;
        private RichTextBox? txtLog;
        private Button? btnConnect, btnDisconnect, btnSwitchMode;
        private Button? btnHeatOn, btnHeatOff, btnCoolOn, btnCoolOff, btnAcidOn, btnAcidOff, btnAlkaliOn, btnAlkaliOff, btnStirOn, btnStirOff;

        public Form1()
        {
            BuildUI();
            this.Load += Form1_Load;
            this.FormClosing += Form1_FormClosing;
        }

        private void BuildUI()
        {
            this.Text = "发酵罐远程监控软件 (自包含)";
            this.Size = new Size(900, 700);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.BackColor = Color.WhiteSmoke;

            Label lblTitle = new Label
            {
                Text = "📡 发酵罐远程监控中心 (自包含模式)",
                Font = new Font("微软雅黑", 16, FontStyle.Bold),
                Location = new Point(20, 20),
                Size = new Size(600, 40),
                ForeColor = Color.DarkBlue
            };
            this.Controls.Add(lblTitle);

            // 传感器区域
            GroupBox groupSensor = new GroupBox
            {
                Text = "发酵罐实时数据",
                Font = new Font("微软雅黑", 10, FontStyle.Bold),
                Location = new Point(20, 80),
                Size = new Size(350, 150)
            };
            lblTempVal = new Label { Text = "25.0 ℃", Location = new Point(90, 30), Size = new Size(100, 25), Font = new Font("微软雅黑", 10, FontStyle.Bold), ForeColor = Color.Red };
            lblPhVal = new Label { Text = "7.0", Location = new Point(90, 65), Size = new Size(100, 25), Font = new Font("微软雅黑", 10, FontStyle.Bold), ForeColor = Color.Blue };
            lblDoVal = new Label { Text = "5.00 mg/L", Location = new Point(90, 100), Size = new Size(100, 25), Font = new Font("微软雅黑", 10, FontStyle.Bold), ForeColor = Color.DarkCyan };
            groupSensor.Controls.AddRange(new Control[] {
                new Label { Text = "温度：", Location = new Point(20, 30), Size = new Size(60, 25) }, lblTempVal,
                new Label { Text = "pH值：", Location = new Point(20, 65), Size = new Size(60, 25) }, lblPhVal,
                new Label { Text = "溶氧量：", Location = new Point(20, 100), Size = new Size(60, 25) }, lblDoVal
            });
            this.Controls.Add(groupSensor);

            // 执行器状态
            GroupBox groupStatus = new GroupBox
            {
                Text = "当前执行器状态",
                Font = new Font("微软雅黑", 10, FontStyle.Bold),
                Location = new Point(400, 80),
                Size = new Size(460, 150)
            };
            lblHeatState = new Label { Text = "加热：● 关闭", Location = new Point(20, 30), Size = new Size(140, 25), ForeColor = Color.Gray };
            lblCoolState = new Label { Text = "冷却：● 关闭", Location = new Point(20, 60), Size = new Size(140, 25), ForeColor = Color.Gray };
            lblAcidState = new Label { Text = "加酸：● 关闭", Location = new Point(200, 30), Size = new Size(120, 25), ForeColor = Color.Gray };
            lblAlkaliState = new Label { Text = "加碱：● 关闭", Location = new Point(200, 60), Size = new Size(120, 25), ForeColor = Color.Gray };
            lblStirState = new Label { Text = "搅拌：● 关闭", Location = new Point(20, 90), Size = new Size(120, 25), ForeColor = Color.Gray };
            lblControlMode = new Label { Text = "控制模式：本地模式", Location = new Point(200, 90), Size = new Size(160, 25), Font = new Font("微软雅黑", 9, FontStyle.Bold), ForeColor = Color.DarkOrange };
            groupStatus.Controls.AddRange(new Control[] { lblHeatState, lblCoolState, lblAcidState, lblAlkaliState, lblStirState, lblControlMode });
            this.Controls.Add(groupStatus);

            // 连接按钮
            btnConnect = new Button { Text = "连接并开始", Location = new Point(20, 260), Size = new Size(120, 35), BackColor = Color.LightGreen };
            btnConnect.Click += BtnConnect_Click;
            btnDisconnect = new Button { Text = "断开连接", Location = new Point(150, 260), Size = new Size(120, 35), Enabled = false, BackColor = Color.LightCoral };
            btnDisconnect.Click += BtnDisconnect_Click;
            this.Controls.AddRange(new Control[] { btnConnect, btnDisconnect });

            // 远程控制区域
            GroupBox groupCtrl = new GroupBox
            {
                Text = "远程控制指令",
                Font = new Font("微软雅黑", 10, FontStyle.Bold),
                Location = new Point(20, 320),
                Size = new Size(840, 150)
            };
            btnSwitchMode = new Button { Text = "切换为远程模式", Location = new Point(20, 30), Size = new Size(140, 35), BackColor = Color.LightBlue };
            btnSwitchMode.Click += (s, e) => ExecuteLocalCommand("ControlMode", 1);

            btnHeatOn = new Button { Text = "加热开", Location = new Point(200, 30), Size = new Size(80, 30), BackColor = Color.Red, ForeColor = Color.White };
            btnHeatOn.Click += (s, e) => ExecuteLocalCommand("HeatState", true);
            btnHeatOff = new Button { Text = "加热关", Location = new Point(290, 30), Size = new Size(80, 30), BackColor = Color.Gray };
            btnHeatOff.Click += (s, e) => ExecuteLocalCommand("HeatState", false);

            btnCoolOn = new Button { Text = "冷却开", Location = new Point(400, 30), Size = new Size(80, 30), BackColor = Color.Cyan };
            btnCoolOn.Click += (s, e) => ExecuteLocalCommand("CoolState", true);
            btnCoolOff = new Button { Text = "冷却关", Location = new Point(490, 30), Size = new Size(80, 30), BackColor = Color.Gray };
            btnCoolOff.Click += (s, e) => ExecuteLocalCommand("CoolState", false);

            btnAcidOn = new Button { Text = "加酸开", Location = new Point(600, 30), Size = new Size(80, 30), BackColor = Color.OrangeRed };
            btnAcidOn.Click += (s, e) => ExecuteLocalCommand("AcidState", true);
            btnAcidOff = new Button { Text = "加酸关", Location = new Point(690, 30), Size = new Size(80, 30), BackColor = Color.Gray };
            btnAcidOff.Click += (s, e) => ExecuteLocalCommand("AcidState", false);

            btnAlkaliOn = new Button { Text = "加碱开", Location = new Point(200, 80), Size = new Size(80, 30), BackColor = Color.Orange };
            btnAlkaliOn.Click += (s, e) => ExecuteLocalCommand("AlkaliState", true);
            btnAlkaliOff = new Button { Text = "加碱关", Location = new Point(290, 80), Size = new Size(80, 30), BackColor = Color.Gray };
            btnAlkaliOff.Click += (s, e) => ExecuteLocalCommand("AlkaliState", false);

            btnStirOn = new Button { Text = "搅拌开", Location = new Point(400, 80), Size = new Size(80, 30), BackColor = Color.Purple, ForeColor = Color.White };
            btnStirOn.Click += (s, e) => ExecuteLocalCommand("StirState", true);
            btnStirOff = new Button { Text = "搅拌关", Location = new Point(490, 80), Size = new Size(80, 30), BackColor = Color.Gray };
            btnStirOff.Click += (s, e) => ExecuteLocalCommand("StirState", false);

            groupCtrl.Controls.AddRange(new Control[] { btnSwitchMode, btnHeatOn, btnHeatOff, btnCoolOn, btnCoolOff, btnAcidOn, btnAcidOff, btnAlkaliOn, btnAlkaliOff, btnStirOn, btnStirOff });
            this.Controls.Add(groupCtrl);

            // 日志
            Label lblLog = new Label { Text = "通信日志", Location = new Point(20, 490), Size = new Size(100, 25), Font = new Font("微软雅黑", 10, FontStyle.Bold) };
            this.Controls.Add(lblLog);
            txtLog = new RichTextBox
            {
                Location = new Point(20, 520),
                Size = new Size(840, 130),
                ReadOnly = true,
                BackColor = Color.Black,
                ForeColor = Color.LightGreen,
                Font = new Font("Consolas", 9)
            };
            this.Controls.Add(txtLog);
        }

        private async void Form1_Load(object sender, EventArgs e)
        {
            AppendLog("程序启动，未连接云端");
        }

        private async void BtnConnect_Click(object sender, EventArgs e)
        {
            try
            {
                var factory = new MqttFactory();
                _mqttClient = factory.CreateMqttClient();

                _mqttClient.DisconnectedAsync += args =>
                {
                    AppendLog($"⚠️ MQTT 连接断开，原因: {args.Reason}");
                    return Task.CompletedTask;
                };

                _mqttClient.ApplicationMessageReceivedAsync += e_args =>
                {
                    string payload = Encoding.UTF8.GetString(e_args.ApplicationMessage.PayloadSegment);
                    AppendLog($"📩 收到消息: {payload}");
                    // 处理云端下发的指令（如果有）
                    HandleCloudCommand(payload);
                    return Task.CompletedTask;
                };

                var options = new MqttClientOptionsBuilder()
                    .WithTcpServer("mqtts.heclouds.com", 1883)
                    .WithClientId(DEVICE_NAME)
                    .WithCredentials(PRODUCT_ID, DEVICE_TOKEN)
                    .WithCleanSession()
                    .WithKeepAlivePeriod(TimeSpan.FromSeconds(120))
                    .Build();

                await _mqttClient.ConnectAsync(options);
                AppendLog("✅ 已连接到 OneNET 平台");

                // 订阅指令下发主题（以便接收平台下发的指令）
                string setTopic = $"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/set";
                await _mqttClient.SubscribeAsync(setTopic);
                AppendLog($"📡 已订阅指令主题: {setTopic}");

                // 订阅回复主题（用于接收上报结果）
                string replyTopic = $"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/post/reply";
                await _mqttClient.SubscribeAsync(replyTopic);
                AppendLog($"📡 已订阅回复主题: {replyTopic}");

                btnConnect!.Enabled = false;
                btnDisconnect!.Enabled = true;

                // 启动定时上报（每5秒上报一次当前模拟数据）
                _timerReport = new System.Windows.Forms.Timer { Interval = 5000 };
                _timerReport.Tick += async (s, ev) => await ReportCurrentState();
                _timerReport.Start();
                AppendLog("已启动定时上报（每5秒）");
            }
            catch (Exception ex)
            {
                AppendLog($"❌ 连接失败: {ex.Message}");
            }
        }

        private async void BtnDisconnect_Click(object sender, EventArgs e)
        {
            if (_mqttClient != null && _mqttClient.IsConnected)
                await _mqttClient.DisconnectAsync();
            _timerReport?.Stop();
            btnConnect!.Enabled = true;
            btnDisconnect!.Enabled = false;
            AppendLog("🔌 已断开连接");
        }

        // 上报当前状态到云端
        private async Task ReportCurrentState()
        {
            if (_mqttClient == null || !_mqttClient.IsConnected)
            {
                AppendLog("未连接，无法上报");
                return;
            }
            long now = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
            string id = now.ToString();  // 纯数字 id

            var payload = new
            {
                id = id,
                version = "1.0",
                @params = new
                {
                    Temperature = new { value = Math.Round(_temperature, 1), time = now },
                    pH = new { value = Math.Round(_ph, 1), time = now },
                    DissolvedOxygen = new { value = Math.Round(_dissolvedOxygen, 2), time = now },
                    HeatState = new { value = _heatState, time = now },
                    CoolState = new { value = _coolState, time = now },
                    AcidState = new { value = _acidState, time = now },
                    AlkaliState = new { value = _alkaliState, time = now },
                    StirState = new { value = _stirState, time = now },
                    ControlMode = new { value = _controlMode, time = now }
                }
            };
            string json = Newtonsoft.Json.JsonConvert.SerializeObject(payload);
            string topic = $"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/post";

            var message = new MqttApplicationMessageBuilder()
                .WithTopic(topic)
                .WithPayload(Encoding.UTF8.GetBytes(json))
                .WithQualityOfServiceLevel(MqttQualityOfServiceLevel.AtLeastOnce)
                .Build();

            await _mqttClient.PublishAsync(message);
            AppendLog($"📤 上报数据: T={_temperature:F1} pH={_ph:F1} DO={_dissolvedOxygen:F2}");
        }

        // 处理云端下发的指令（例如从平台手动下发）
        private void HandleCloudCommand(string payload)
        {
            try
            {
                var json = JObject.Parse(payload);
                var p = json["params"];
                if (p == null) return;

                // 根据云端指令修改本地状态
                if (p["ControlMode"] != null) _controlMode = p["ControlMode"]?["value"]?.Value<int>() ?? _controlMode;
                if (p["HeatState"] != null) _heatState = p["HeatState"]?["value"]?.Value<bool>() ?? _heatState;
                if (p["CoolState"] != null) _coolState = p["CoolState"]?["value"]?.Value<bool>() ?? _coolState;
                if (p["AcidState"] != null) _acidState = p["AcidState"]?["value"]?.Value<bool>() ?? _acidState;
                if (p["AlkaliState"] != null) _alkaliState = p["AlkaliState"]?["value"]?.Value<bool>() ?? _alkaliState;
                if (p["StirState"] != null) _stirState = p["StirState"]?["value"]?.Value<bool>() ?? _stirState;

                this.Invoke(new Action(UpdateUI));
                AppendLog("云端指令已执行，界面已更新");
            }
            catch (Exception ex)
            {
                AppendLog($"处理云端指令出错: {ex.Message}");
            }
        }

        // 本地控制指令（直接修改状态并上报）
        private async void ExecuteLocalCommand(string property, object value)
        {
            // 如果处于远程模式，则通过MQTT发送指令（让云端下发给本设备，但也会触发本地？为简单，直接修改状态）
            // 这里我们直接修改本地状态，然后上报，模拟控制效果
            switch (property)
            {
                case "ControlMode": _controlMode = (int)value; break;
                case "HeatState": _heatState = (bool)value; break;
                case "CoolState": _coolState = (bool)value; break;
                case "AcidState": _acidState = (bool)value; break;
                case "AlkaliState": _alkaliState = (bool)value; break;
                case "StirState": _stirState = (bool)value; break;
            }
            UpdateUI();
            AppendLog($"本地指令: {property} = {value}，状态已更新");
            // 立即上报一次新状态
            if (_mqttClient != null && _mqttClient.IsConnected)
                await ReportCurrentState();
            else
                AppendLog("未连接云端，状态仅本地更新");
        }

        private void UpdateUI()
        {
            if (lblTempVal != null) lblTempVal.Text = $"{_temperature:F1} ℃";
            if (lblPhVal != null) lblPhVal.Text = $"{_ph:F1}";
            if (lblDoVal != null) lblDoVal.Text = $"{_dissolvedOxygen:F2} mg/L";

            if (lblHeatState != null)
            {
                lblHeatState.Text = _heatState ? "加热：● 开启" : "加热：● 关闭";
                lblHeatState.ForeColor = _heatState ? Color.Red : Color.Gray;
            }
            if (lblCoolState != null)
            {
                lblCoolState.Text = _coolState ? "冷却：● 开启" : "冷却：● 关闭";
                lblCoolState.ForeColor = _coolState ? Color.Cyan : Color.Gray;
            }
            if (lblAcidState != null)
            {
                lblAcidState.Text = _acidState ? "加酸：● 开启" : "加酸：● 关闭";
                lblAcidState.ForeColor = _acidState ? Color.OrangeRed : Color.Gray;
            }
            if (lblAlkaliState != null)
            {
                lblAlkaliState.Text = _alkaliState ? "加碱：● 开启" : "加碱：● 关闭";
                lblAlkaliState.ForeColor = _alkaliState ? Color.Orange : Color.Gray;
            }
            if (lblStirState != null)
            {
                lblStirState.Text = _stirState ? "搅拌：● 开启" : "搅拌：● 关闭";
                lblStirState.ForeColor = _stirState ? Color.Purple : Color.Gray;
            }
            if (lblControlMode != null)
            {
                lblControlMode.Text = _controlMode == 0 ? "控制模式：本地模式" : "控制模式：远程模式";
                lblControlMode.ForeColor = _controlMode == 0 ? Color.Green : Color.DarkOrange;
            }
        }

        private void AppendLog(string msg)
        {
            if (InvokeRequired)
            {
                Invoke(new Action(() => AppendLog(msg)));
                return;
            }
            txtLog?.AppendText($"[{DateTime.Now:HH:mm:ss}] {msg}{Environment.NewLine}");
            txtLog?.ScrollToCaret();
        }

        private async void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            _timerReport?.Stop();
            if (_mqttClient != null && _mqttClient.IsConnected)
                await _mqttClient.DisconnectAsync();
        }
    }
}