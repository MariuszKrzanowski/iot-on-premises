using MQTTnet.Client;
using System;
using System.Threading;
using System.Threading.Tasks;

namespace MrMatrix.Net.IoTOnPremises.OffLineVoiceDemo
{
    class Program
    {
        static async Task Main(string[] args)
        {
            Console.WriteLine("Initialize.");
            using (Runner runner = new Runner())
            {
                Console.WriteLine("Press ENTER to listen.");
                Console.ReadLine();
                await runner.Run();
                Console.WriteLine("Press ENTER to exit");
                Console.ReadLine();
                Console.WriteLine("Cleaning process started.");
            }
            await Task.Delay(TimeSpan.FromSeconds(2));
            Console.WriteLine("Cleanup complete.");
        }
    }
}
