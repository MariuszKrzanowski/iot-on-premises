using System;
using System.Threading.Tasks;
using MrMatrix.Net.IoTOnPremises.OffLineVoiceDemo.Mqtt;
using MrMatrix.Net.IoTOnPremises.OffLineVoiceDemo.Speech;

namespace MrMatrix.Net.IoTOnPremises.OffLineVoiceDemo
{
    public sealed class Runner : IDisposable
    {
        private readonly MqttRunner _mqttRunner;
        private readonly SpeechRecognitionRunner _speechRecognitionRunner;

        public Runner()
        {
            _mqttRunner = new MqttRunner();
            _speechRecognitionRunner=new SpeechRecognitionRunner();
            _speechRecognitionRunner.MessageRecognized += _speechRecognitionRunner_MessageRecognized;


        }

        private void _speechRecognitionRunner_MessageRecognized(object sender, Message e)
        {
            _mqttRunner.PublishAsync(e);
        }

        public async Task Run()
        {
            await _mqttRunner.Run();
            _speechRecognitionRunner.Run();
        }
        
        public void Dispose()
        {
            _speechRecognitionRunner?.Dispose();
            _mqttRunner?.Dispose();
        }
    }
}