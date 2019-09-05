using Microsoft.CognitiveServices.Speech;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Options;
using System;
using System.Threading.Tasks;
using MrMatrix.Net.IoTOnPremises.VoiceDemo.Properties;

namespace MrMatrix.Net.IoTOnPremises.VoiceDemo
{
    class Program
    {
        public static async Task RecognizeSpeechAsync()
        {
#error UPDATE CONFIGURATION FIRST!!!
            // Based on sample from MSDN.
            // Creates an instance of a speech config with specified subscription key and service region.
            // Replace with your own subscription key
            // and service region (e.g., "westus").
            var config = SpeechConfig.FromSubscription(Settings.Default.CognitiveServicesKey, Settings.Default.CognitiveServicesRegion);
            using (SpeechSynthesizer synthesizer = new SpeechSynthesizer(config))
            {
                await synthesizer.SpeakTextAsync("Hello Aragorn!");
            }

            // Creates a speech recognizer.
            using (var recognizer = new SpeechRecognizer(config))
            {
                recognizer.Recognized += Recognizer_Recognized;
                recognizer.Recognizing += Recognizer_Recognizing;
                await recognizer.StartContinuousRecognitionAsync();

                Console.WriteLine("Please press a key to continue.");
                Console.ReadLine();
                await recognizer.StopContinuousRecognitionAsync();
            }
        }

        private static void Recognizer_Recognizing(object sender, SpeechRecognitionEventArgs e)
        {
            //Console.ForegroundColor = ConsoleColor.Gray;
            //Console.WriteLine();
            //Console.WriteLine($"SpeechRecognitionEventArgs={e.Result}");
            //Console.WriteLine($"SpeechRecognitionEventArgs={e.Result.Text}");
        }

        private static void Recognizer_Recognized(object sender, SpeechRecognitionEventArgs e)
        {
            Console.ForegroundColor = ConsoleColor.DarkYellow;
            Console.WriteLine();
            Console.WriteLine($"SpeechRecognitionEventArgs={e.Result}");
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine($"SpeechRecognitionEventArgs={e.Result.Text}");

            string recognizedText = e.Result.Text;

            if (string.IsNullOrWhiteSpace(recognizedText) || recognizedText.Length < 10)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("Command ignored");
                return;
            }

            recognizedText = recognizedText.ToLower();

            bool redExists = recognizedText.Contains("red");
            bool yellowExists = recognizedText.Contains("yellow");
            bool greenExists = recognizedText.Contains("green");

            bool officeExists = recognizedText.Contains("office");
            bool kitchenExists = recognizedText.Contains("kitchen");

            bool lightExists = recognizedText.Contains("light");

            bool switchOnExists = recognizedText.Contains("switch on");
            bool switchOffExists = recognizedText.Contains("switch off");

            if (!lightExists)
            {

                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("Command ignored");
                return; 
            }

            string command;

            if (switchOnExists)
            {
                command = "HIGH";
            }
            else if (switchOffExists)
            {
                command = "LOW";
            }
            else
            {
                return;
            }


            var topic = "/light/command/";
            if (redExists)
            {
                topic += "lightRed";
            }
            else if (yellowExists)
            {
                topic += "lightYellow";
            }
            else if (greenExists)
            {
                topic += "lightGreen";

            }
            else
            {
                topic = "/socket/command/";
                if (officeExists)
                {
                    topic += "sonoff1";
                }
                else if (kitchenExists)
                {
                    topic += "sonoff2";
                }
                else
                {
                    return;
                }
            }

            Console.WriteLine();
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine($"SUCCESS!!!");
            Console.WriteLine(e.Result.Text);
            Console.WriteLine($"topic=[{topic}]   command=[{command}]");
            Console.WriteLine();

            client.PublishAsync(topic, System.Text.Encoding.ASCII.GetBytes(command));
        }

        private static IMqttClient client;

        static async Task Main()
        {
            var clientOptions = new MqttClientOptionsBuilder()
                .WithClientId("VoiceCommand")
                .WithTcpServer(Settings.Default.MqttServer)
                .WithCredentials(Settings.Default.MqttUser, Settings.Default.MqttPassword)
                .WithCleanSession()
                .Build();

            var factory = new MqttFactory();

            client = factory.CreateMqttClient();
            client.UseDisconnectedHandler((args) =>
            {
                return Task.Run(async () =>
                {
                    if (args.ClientWasConnected)
                    {
                        await Task.Delay(5000);
                        await client.ReconnectAsync();
                    }
                });
            });

            client.UseConnectedHandler((c) => Task.Run(() =>
            {
                Console.WriteLine($"MQTT Connected! {c.AuthenticateResult.ResultCode}");
            }));

            await client.ConnectAsync(clientOptions).ConfigureAwait(false);

            await RecognizeSpeechAsync();
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine("Please ENTER key to exit.");
            Console.ReadLine();
        }
    }
}
