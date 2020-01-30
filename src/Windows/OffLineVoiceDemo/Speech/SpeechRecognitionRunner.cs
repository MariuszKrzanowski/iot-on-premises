using System;
using System.Speech.Recognition;
using MrMatrix.Net.IoTOnPremises.OffLineVoiceDemo.Mqtt;

namespace MrMatrix.Net.IoTOnPremises.OffLineVoiceDemo.Speech
{


    public sealed class SpeechRecognitionRunner : IDisposable
    {


        private VoiceCommandState _voiceCommandState;
        private readonly SpeechRecognitionEngine _recognizer;
        private readonly SentenceParser _sentenceParser;
        public SpeechRecognitionRunner()
        {
            _voiceCommandState = VoiceCommandState.ListenModeOff;
            _sentenceParser = new SentenceParser();
            _recognizer = new SpeechRecognitionEngine();

            var choicesOnOff = new Choices(new[] { GrammarDictionary.ChoiceOnOff.On, GrammarDictionary.ChoiceOnOff.Off });
            var choicesSwitchTurn = new Choices(new[] { GrammarDictionary.SwitchTurn.Switch, GrammarDictionary.SwitchTurn.Turn });
            _recognizer.LoadGrammar(CreateActivationGrammar(choicesOnOff));
            _recognizer.LoadGrammar(CreateSonoffGrammar(choicesSwitchTurn, choicesOnOff));
            _recognizer.LoadGrammar(CreateDiodesGrammar(choicesSwitchTurn, choicesOnOff));

            _recognizer.SpeechRecognized += Recognizer_SpeechRecognized;
            _recognizer.AudioStateChanged += Recognizer_AudioStateChanged;
            _recognizer.SpeechHypothesized += Recognizer_SpeechHypothesized;
            _recognizer.SetInputToDefaultAudioDevice();
        }

        private static Grammar CreateActivationGrammar(Choices choicesOnOff)
        {
            var activationPhrase = new GrammarBuilder();
            activationPhrase.Append(GrammarDictionary.WakeupWord);
            activationPhrase.Append(GrammarDictionary.ListenMode);
            activationPhrase.Append(choicesOnOff);
            var activationGrammar = new Grammar(activationPhrase);
            return activationGrammar;
        }

        private static Grammar CreateSonoffGrammar(Choices choicesSwitchTurn, Choices choicesOnOff)
        {
            var choicesPlaces = new Choices(new[] { GrammarDictionary.Places.Kitchen, GrammarDictionary.Places.Office, GrammarDictionary.Places.LivingRoom });

            var sonoffGrammarBuilder = new GrammarBuilder();
            sonoffGrammarBuilder.Append(choicesSwitchTurn);
            sonoffGrammarBuilder.Append(choicesOnOff);

            sonoffGrammarBuilder.Append(GrammarDictionary.Light);
            sonoffGrammarBuilder.Append(GrammarDictionary.In);
            sonoffGrammarBuilder.Append(GrammarDictionary.The);
            sonoffGrammarBuilder.Append(choicesPlaces);
            var sonoffGrammar = new Grammar(sonoffGrammarBuilder);
            return sonoffGrammar;
        }

        private static Grammar CreateDiodesGrammar(Choices choicesSwitchTurn, Choices choicesOnOff)
        {
            var choicesColor = new Choices(new[] { GrammarDictionary.ChoiceColor.Red, GrammarDictionary.ChoiceColor.Green, GrammarDictionary.ChoiceColor.Yellow });
            
            var diodesGrammarBuilder = new GrammarBuilder();
            diodesGrammarBuilder.Append(choicesSwitchTurn);
            diodesGrammarBuilder.Append(choicesOnOff);

            diodesGrammarBuilder.Append(GrammarDictionary.The);
            diodesGrammarBuilder.Append(choicesColor);
            diodesGrammarBuilder.Append(GrammarDictionary.Light);
            var diodesGrammar = new Grammar(diodesGrammarBuilder);
            return diodesGrammar;
        }

        public void Run()
        {
            _recognizer.SetInputToDefaultAudioDevice();
            // Start asynchronous, continuous speech recognition.  
            _recognizer.RecognizeAsync(RecognizeMode.Multiple);
        }

        public event EventHandler<Message> MessageRecognized;

        private void Recognizer_AudioStateChanged(object sender, AudioStateChangedEventArgs e)
        {
            Console.ForegroundColor = ConsoleColor.DarkGray;
            Console.WriteLine($"STATE={e.AudioState}");
            Console.ForegroundColor = ConsoleColor.Gray;
        }

        private void Recognizer_SpeechHypothesized(object sender, SpeechHypothesizedEventArgs e)
        {
            Console.ForegroundColor = ConsoleColor.DarkGray;
            Console.WriteLine($"Hypothesized {e.Result.Text}");
            Console.ForegroundColor = ConsoleColor.Gray;
        }

        private void Recognizer_SpeechRecognized(object sender, SpeechRecognizedEventArgs e)
        {
            if (_voiceCommandState == VoiceCommandState.ListenModeOff && (e.Result.Text == $"{GrammarDictionary.WakeupWord} {GrammarDictionary.ListenMode} {GrammarDictionary.ChoiceOnOff.On}"))
            {
                Console.Beep(500, 300);
                Console.Beep(700, 300);
                Console.ForegroundColor = ConsoleColor.Yellow;
                _voiceCommandState = VoiceCommandState.ListenModeOn;
                Console.WriteLine($"Voice command state {_voiceCommandState}");
                Console.ForegroundColor = ConsoleColor.Gray;
                return;
            }

            if (_voiceCommandState == VoiceCommandState.ListenModeOn && e.Result.Text == $"{GrammarDictionary.WakeupWord} {GrammarDictionary.ListenMode} {GrammarDictionary.ChoiceOnOff.Off}")
            {
                Console.Beep(600, 300);
                Console.Beep(300, 300);

                Console.ForegroundColor = ConsoleColor.Red;
                _voiceCommandState = VoiceCommandState.ListenModeOff;
                Console.WriteLine($"Voice command state {_voiceCommandState}");
                Console.ForegroundColor = ConsoleColor.Gray;
                return;
            }

            if (_voiceCommandState == VoiceCommandState.ListenModeOff)
            {
                return;
            }

            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine($"Recognized {e.Result.Text}");
            Message message = _sentenceParser.AnalyzeRecognizedText(e.Result.Text);
            if (message != null)
            {
                Console.WriteLine();
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine($"SUCCESS!!!");
                Console.WriteLine(e.Result.Text);
                Console.Write($"topic=[{message.Topic}]   command=[");
                Console.ForegroundColor = "HIGH".Equals(message.Command) ? ConsoleColor.Yellow : ConsoleColor.Red;
                Console.Write(message.Command);
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("]");
                Console.WriteLine();
                this.MessageRecognized?.Invoke(this, message);
                Console.Beep(400, 200);
            }
            Console.ForegroundColor = ConsoleColor.Gray;
        }

        public void Dispose()
        {
            _recognizer?.RecognizeAsyncStop();
            _recognizer?.Dispose();
        }
    }
}