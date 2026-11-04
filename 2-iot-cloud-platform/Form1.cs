using System;
using System.Drawing;
using System.Threading.Tasks;
using System.Windows.Forms;
using Newtonsoft.Json.Linq;

namespace IOTAPP_Simulator
{
    public class Form1 : Form
    {
        // 请替换为您的真实信息
        private const string PRODUCT_ID = "YOUR_PRODUCT_ID";
        private const string DEVICE_NAME = "IOTAPP_Simulator";
        private const string DEVICE_SECRET = "";

        private OneMqttClient? _mqttClient;
        private System.Windows.Forms.Timer? _timerSensor;
        private System.Windows.Forms.Timer? _timerSimulation;
        private System.Windows.Forms.Timer? _timerHeartbeat;
        private Random _rand = new Random();

        private double _temperature = 25.0;
        private double _ph = 7.0;
        private double _dissolvedOxygen = 5.0;
        private bool _heatState = false;
        private bool _coolState = false;
        private bool _acidState = false;
        private bool _alkaliState = false;
        private bool _stirState = false;
        private int _controlMode = 0;

        private Label lblTempValue, lblPhValue, lblDoValue;
        private Label lblHeatState, lblCoolState, lblAcidState, lblAlkaliState, lblStirState, lblControlMode;
        private RichTextBox txtLog;
        private Button btnConnect, btnDisconnect, btnLocalMode, btnRemoteMode, btnGenerateData;

        public Form1()
        {
            BuildUI();
            this.Load += Form1_Load;
            this.FormClosing += Form1_FormClosing;
        }

        private void BuildUI()
        {
            this.Text = "发酵罐仿真软件";
            this.Size = new Size(800, 600);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.BackColor = Color.WhiteSmoke;

            Label lblTitle = new Label { Text = "🧪 发酵罐仿真软件", Font = new Font("微软雅黑", 16, FontStyle.Bold), Location = new Point(20, 20), Size = new Size(300, 40), ForeColor = Color.DarkGreen };
            this.Controls.Add(lblTitle);

            GroupBox groupSensor = new GroupBox { Text = "实时传感器数据", Font = new Font("微软雅黑", 10, FontStyle.Bold), Location = new Point(20, 80), Size = new Size(350, 150) };
            lblTempValue = new Label { Text = "25.0 ℃", Location = new Point(90, 30), Size = new Size(100, 25), Font = new Font("微软雅黑", 10, FontStyle.Bold), ForeColor = Color.Red };
            lblPhValue = new Label { Text = "7.0", Location = new Point(90, 65), Size = new Size(100, 25), Font = new Font("微软雅黑", 10, FontStyle.Bold), ForeColor = Color.Blue };
            lblDoValue = new Label { Text = "5.00 mg/L", Location = new Point(90, 100), Size = new Size(100, 25), Font = new Font("微软雅黑", 10, FontStyle.Bold), ForeColor = Color.DarkCyan };
            groupSensor.Controls.AddRange(new Control[] { new Label { Text = "温度：", Location = new Point(20, 30), Size = new Size(60, 25) }, lblTempValue,
                                                           new Label { Text = "pH值：", Location = new Point(20, 65), Size = new Size(60, 25) }, lblPhValue,
                                                           new Label { Text = "溶氧量：", Location = new Point(20, 100), Size = new Size(60, 25) }, lblDoValue });
            this.Controls.Add(groupSensor);

            GroupBox groupActuator = new GroupBox { Text = "执行器状态", Font = new Font("微软雅黑", 10, FontStyle.Bold), Location = new Point(400, 80), Size = new Size(350, 150) };
            lblHeatState = new Label { Text = "加热：● 关闭", Location = new Point(20, 30), Size = new Size(120, 25), ForeColor = Color.Gray };
            lblCoolState = new Label { Text = "冷却：● 关闭", Location = new Point(20, 60), Size = new Size(120, 25), ForeColor = Color.Gray };
            lblAcidState = new Label { Text = "加酸：● 关闭", Location = new Point(180, 30), Size = new Size(120, 25), ForeColor = Color.Gray };
            lblAlkaliState = new Label { Text = "加碱：● 关闭", Location = new Point(180, 60), Size = new Size(120, 25), ForeColor = Color.Gray };
            lblStirState = new Label { Text = "搅拌：● 关闭", Location = new Point(20, 90), Size = new Size(120, 25), ForeColor = Color.Gray };
            lblControlMode = new Label { Text = "控制模式：本地模式", Location = new Point(180, 90), Size = new Size(160, 25), Font = new Font("微软雅黑", 9, FontStyle.Bold), ForeColor = Color.DarkOrange };
            groupActuator.Controls.AddRange(new Control[] { lblHeatState, lblCoolState, lblAcidState, lblAlkaliState, lblStirState, lblControlMode });
            this.Controls.Add(groupActuator);

            btnConnect = new Button { Text = "连接 OneNET", Location = new Point(20, 260), Size = new Size(120, 35), BackColor = Color.LightGreen };
            btnConnect.Click += BtnConnect_Click;
            btnDisconnect = new Button { Text = "断开连接", Location = new Point(160, 260), Size = new Size(120, 35), Enabled = false, BackColor = Color.LightCoral };
            btnDisconnect.Click += BtnDisconnect_Click;
            btnLocalMode = new Button { Text = "本地模式", Location = new Point(20, 310), Size = new Size(120, 35), BackColor = Color.LightBlue };
            btnLocalMode.Click += (s, e) => SwitchMode(0);
            btnRemoteMode = new Button { Text = "远程模式", Location = new Point(160, 310), Size = new Size(120, 35), BackColor = Color.LightBlue };
            btnRemoteMode.Click += (s, e) => SwitchMode(1);
            btnGenerateData = new Button { Text = "随机生成传感器值", Location = new Point(300, 260), Size = new Size(150, 35), BackColor = Color.LightYellow };
            btnGenerateData.Click += BtnGenerateData_Click;
            this.Controls.AddRange(new Control[] { btnConnect, btnDisconnect, btnLocalMode, btnRemoteMode, btnGenerateData });

            Label lblLog = new Label { Text = "运行日志", Location = new Point(20, 360), Size = new Size(100, 25), Font = new Font("微软雅黑", 10, FontStyle.Bold) };
            this.Controls.Add(lblLog);
            txtLog = new RichTextBox { Location = new Point(20, 390), Size = new Size(730, 160), ReadOnly = true, BackColor = Color.Black, ForeColor = Color.LightGreen, Font = new Font("Consolas", 9) };
            this.Controls.Add(txtLog);
        }

        private void SwitchMode(int mode)
        {
            _controlMode = mode;
            UpdateUI();
            AppendLog(mode == 0 ? "🔘 切换到本地控制模式" : "🔘 切换到远程控制模式");
        }

        private void UpdateUI()
        {
            lblTempValue.Text = $"{_temperature:F1} ℃";
            lblPhValue.Text = $"{_ph:F1}";
            lblDoValue.Text = $"{_dissolvedOxygen:F2} mg/L";

            lblHeatState.Text = _heatState ? "加热：● 开启" : "加热：● 关闭";
            lblHeatState.ForeColor = _heatState ? Color.Red : Color.Gray;
            lblCoolState.Text = _coolState ? "冷却：● 开启" : "冷却：● 关闭";
            lblCoolState.ForeColor = _coolState ? Color.Cyan : Color.Gray;
            lblAcidState.Text = _acidState ? "加酸：● 开启" : "加酸：● 关闭";
            lblAcidState.ForeColor = _acidState ? Color.OrangeRed : Color.Gray;
            lblAlkaliState.Text = _alkaliState ? "加碱：● 开启" : "加碱：● 关闭";
            lblAlkaliState.ForeColor = _alkaliState ? Color.Orange : Color.Gray;
            lblStirState.Text = _stirState ? "搅拌：● 开启" : "搅拌：● 关闭";
            lblStirState.ForeColor = _stirState ? Color.Purple : Color.Gray;
            lblControlMode.Text = _controlMode == 0 ? "控制模式：本地模式" : "控制模式：远程模式";
            lblControlMode.ForeColor = _controlMode == 0 ? Color.Green : Color.DarkOrange;
        }

        private async void Form1_Load(object sender, EventArgs e) => AppendLog("程序启动，等待连接...");

        private async void BtnConnect_Click(object sender, EventArgs e)
        {
            try
            {
                _mqttClient = new OneMqttClient(PRODUCT_ID, DEVICE_NAME, DEVICE_SECRET);
                await _mqttClient.ConnectAsync();
                AppendLog("✅ 已连接到 OneNET 平台");

                string setTopic = $"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/set";
                await _mqttClient.SubscribeAsync(setTopic);
                AppendLog($"📡 已订阅指令主题: {setTopic}");


                string replyTopic = $"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/post/reply";
                await _mqttClient.SubscribeAsync(replyTopic);
                AppendLog($"📡 已订阅属性上报结果回复主题: {replyTopic}");






                _mqttClient.OnMessageReceived += OnMessageReceived;

                _timerSensor = new System.Windows.Forms.Timer { Interval = 5000 };
                _timerSensor.Tick += async (s, ev) => await UploadSensorData();
                _timerSensor.Start();

                _timerSimulation = new System.Windows.Forms.Timer { Interval = 1000 };
                _timerSimulation.Tick += (s, ev) => UpdateSimulation();
                _timerSimulation.Start();

                _timerHeartbeat = new System.Windows.Forms.Timer { Interval = 30000 };
                _timerHeartbeat.Tick += async (s, ev) => await SendHeartbeat();
                _timerHeartbeat.Start();

                btnConnect.Enabled = false;
                btnDisconnect.Enabled = true;
            }
            catch (Exception ex) { AppendLog($"❌ 连接失败: {ex.Message}"); }
        }

        private async void BtnDisconnect_Click(object sender, EventArgs e)
        {
            if (_mqttClient != null)
            {
                _mqttClient.OnMessageReceived -= OnMessageReceived;
                if (_mqttClient.IsConnected) await _mqttClient.DisconnectAsync();
            }
            _timerSensor?.Stop();
            _timerSimulation?.Stop();
            _timerHeartbeat?.Stop();
            btnConnect.Enabled = true;
            btnDisconnect.Enabled = false;
            AppendLog("🔌 已断开连接");
        }

        private void UpdateSimulation()
        {
            _temperature += (_rand.NextDouble() - 0.5) * 0.2;
            _temperature = Math.Clamp(_temperature, 20, 80);
            _ph += (_rand.NextDouble() - 0.5) * 0.1;
            _ph = Math.Clamp(_ph, 3, 9);
            _dissolvedOxygen += (_rand.NextDouble() - 0.5) * 0.1;
            _dissolvedOxygen = Math.Clamp(_dissolvedOxygen, 0, 10);

            if (_controlMode == 0) ApplyFuzzyControl();

            this.Invoke(new Action(() => UpdateUI()));
        }

        private void ApplyFuzzyControl()
        {
            const double targetTemp = 37.0;
            double error = targetTemp - _temperature;
            if (Math.Abs(error) < 0.5) return;
            if (error > 0) { _heatState = true; _coolState = false; AppendLog($"🌡️ 模糊控制: 温度偏低({_temperature:F1}℃)，开启加热"); }
            else { _heatState = false; _coolState = true; AppendLog($"🌡️ 模糊控制: 温度偏高({_temperature:F1}℃)，开启冷却"); }
        }




        private async Task UploadSensorData()
        {
            long now = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
            string numericId = now.ToString();   // 纯数字 id

            var data = new
            {
                id = numericId,
                version = "1.0",
                @params = new
                {
                    Temperature = new { value = Math.Round(_temperature, 1), time = now },
                    pH = new { value = Math.Round(_ph, 1), time = now },
                    DissolvedOxygen = new { value = Math.Round(_dissolvedOxygen, 2), time = now },
                    // 关键修改：直接使用布尔变量，不要用 1/0
                    HeatState = new { value = _heatState, time = now },
                    CoolState = new { value = _coolState, time = now },
                    AcidState = new { value = _acidState, time = now },
                    AlkaliState = new { value = _alkaliState, time = now },
                    StirState = new { value = _stirState, time = now },
                    ControlMode = new { value = _controlMode, time = now }
                }
            };

            string topic = $"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/post";
            await _mqttClient!.PublishAsync(topic, data);
            AppendLog($"📤 上报数据: T={_temperature:F1}℃ pH={_ph:F1} DO={_dissolvedOxygen:F2} (id={numericId})");
        }








        private async Task SendHeartbeat()
        {
            var heartbeat = new { id = Guid.NewGuid().ToString(), version = "1.0", @params = new { Heartbeat = new { value = DateTimeOffset.UtcNow.ToUnixTimeSeconds() } } };
            string topic = $"$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/post";
            await _mqttClient!.PublishAsync(topic, heartbeat);
        }

        private async Task OnMessageReceived(string payload)
        {
            AppendLog($"📩 收到指令: {payload}");
            try
            {
                var json = JObject.Parse(payload);



                if (json["code"] != null)
                {
                    int code = (int)json["code"];
                    string msg = json["msg"]?.ToString() ?? "";
                    AppendLog($"📋 平台回复: code={code}, msg={msg}");
                    if (code == 200)
                        AppendLog("✅ 数据上报成功");
                    else
                        AppendLog($"❌ 数据上报失败: {msg}");
                    return;  // 回复消息不继续处理控制指令
                }




                var p = json["params"];
                if (p == null) return;
                if (p["ControlMode"] != null) _controlMode = (int)p["ControlMode"]["value"];
                if (_controlMode == 1)
                {
                    if (p["HeatState"] != null) _heatState = (int)p["HeatState"]["value"] == 1;
                    if (p["CoolState"] != null) _coolState = (int)p["CoolState"]["value"] == 1;
                    if (p["AcidState"] != null) _acidState = (int)p["AcidState"]["value"] == 1;
                    if (p["AlkaliState"] != null) _alkaliState = (int)p["AlkaliState"]["value"] == 1;
                    if (p["StirState"] != null) _stirState = (int)p["StirState"]["value"] == 1;
                    this.Invoke(new Action(() => UpdateUI()));
                    AppendLog($"🎮 远程指令已执行");
                }
                else AppendLog("⚠️ 本地模式，忽略远程指令");
            }
            catch (Exception ex) { AppendLog($"❌ 解析指令错误: {ex.Message}"); }
        }

        private void BtnGenerateData_Click(object sender, EventArgs e)
        {
            _temperature = 20 + _rand.NextDouble() * 60;
            _ph = 3 + _rand.NextDouble() * 6;
            _dissolvedOxygen = _rand.NextDouble() * 10;
            UpdateUI();
            AppendLog($"🔄 手动生成传感器值: T={_temperature:F1} pH={_ph:F1} DO={_dissolvedOxygen:F2}");
        }

        private void AppendLog(string msg)
        {
            if (this.InvokeRequired) { this.Invoke(new Action(() => AppendLog(msg))); return; }
            txtLog.AppendText($"[{DateTime.Now:HH:mm:ss}] {msg}{Environment.NewLine}");
            txtLog.ScrollToCaret();
        }

        private async void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            _timerSensor?.Stop();
            _timerSimulation?.Stop();
            _timerHeartbeat?.Stop();
            if (_mqttClient != null)
            {
                _mqttClient.OnMessageReceived -= OnMessageReceived;
                if (_mqttClient.IsConnected) await _mqttClient.DisconnectAsync();
            }
        }
    }
}