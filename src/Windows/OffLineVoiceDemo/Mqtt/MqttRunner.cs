using System;
using System.Threading.Tasks;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Options;
using MrMatrix.Net.IoTOnPremises.OffLineVoiceDemo.Properties;

namespace MrMatrix.Net.IoTOnPremises.OffLineVoiceDemo.Mqtt
{
    public sealed class MqttRunner : IDisposable
    {
        private readonly IMqttClient _client;

        public MqttRunner()
        {
            var factory = new MqttFactory();

            _client = factory.CreateMqttClient();
            _client.UseDisconnectedHandler((disconnectArgs) =>
            {
                return Task.Run(async () =>
                {
                    if (disconnectArgs.ClientWasConnected)
                    {
                        await Task.Delay(10000);
                        await _client.ReconnectAsync();
                    }
                });
            });

            _client.UseConnectedHandler((c) => Task.Run(() =>
            {
                Console.WriteLine($"MQTT Connected! {c.AuthenticateResult.ResultCode}");
            }));
        }

        public async Task Run()
        {
            var clientOptions = new MqttClientOptionsBuilder()
                .WithClientId("VoiceCommand")
                .WithTcpServer(Settings.Default.MqttServer)
                .WithCredentials(Settings.Default.MqttUser, Settings.Default.MqttPassword)
                .WithCleanSession()
                .Build();

            try
            {
                await _client.ConnectAsync(clientOptions).ConfigureAwait(false);
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
        }

        public async Task PublishAsync(Message message)
        {
            try
            {
                if (_client.IsConnected)
                {
                    await _client.PublishAsync(message.Topic, System.Text.Encoding.ASCII.GetBytes(message.Command)).ConfigureAwait(false);
                }
            }
            catch (Exception exception)
            {
                Console.WriteLine(exception);
            }
        }

        public void Dispose()
        {
            _client?.Dispose();
        }

    }
}