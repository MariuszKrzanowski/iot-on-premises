using System;
using MrMatrix.Net.IoTOnPremises.OffLineVoiceDemo.Mqtt;

namespace MrMatrix.Net.IoTOnPremises.OffLineVoiceDemo.Speech
{
    public class SentenceParser
    {
        private const int MinimumSentenceWidth = 10;

       

        public SentenceParser()
        {

        }

        public Message AnalyzeRecognizedText(string originalRecognizedText)
        {
            if (string.IsNullOrWhiteSpace(originalRecognizedText) || originalRecognizedText.Length < MinimumSentenceWidth)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("Command ignored");
                return null;
            }

            var recognized = new RecognizedPatterns(originalRecognizedText.ToLower());

           
            if (!recognized.LightExists)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("Command ignored");
                return null;
            }

            Message message = new Message();

            if (recognized.SwitchOnExists)
            {
                message.Command = "HIGH";
            }
            else if (recognized.SwitchOffExists)
            {
                message.Command = "LOW";
            }
            else
            {
                return null;
            }


            message.Topic = "/light/command/";
            if (recognized.RedExists)
            {
                message.Topic += "lightRed";
            }
            else if (recognized.YellowExists)
            {
                message.Topic += "lightYellow";
            }
            else if (recognized.GreenExists)
            {
                message.Topic += "lightGreen";

            }
            else
            {
                message.Topic = "/socket/command/";
                if (recognized.OfficeExists)
                {
                    message.Topic += "sonoff1";
                }
                else if (recognized.KitchenExists)
                {
                    message.Topic += "sonoff2";
                }
                else
                {
                    return null;
                }
            }

            return message;
        }

    }
}