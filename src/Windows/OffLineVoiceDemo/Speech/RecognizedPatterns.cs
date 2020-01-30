namespace MrMatrix.Net.IoTOnPremises.OffLineVoiceDemo.Speech
{
    public class RecognizedPatterns
    {
        private readonly string _text;

        public RecognizedPatterns(string text)
        {
            _text = text;
        }

        public bool RedExists => _text.Contains(GrammarDictionary.ChoiceColor.Red);
        public bool YellowExists =>
            _text.Contains(GrammarDictionary.ChoiceColor.Yellow);
        public bool GreenExists => _text.Contains(GrammarDictionary.ChoiceColor.Green);
        public bool OfficeExists => _text.Contains(GrammarDictionary.Places.Office);
        public bool KitchenExists => _text.Contains(GrammarDictionary.Places.Kitchen);
        public bool LightExists => _text.Contains(GrammarDictionary.Light);
        public bool SwitchOnExists => 
            _text.Contains($"{GrammarDictionary.SwitchTurn.Switch} {GrammarDictionary.ChoiceOnOff.On}") 
            || 
            _text.Contains($"{GrammarDictionary.SwitchTurn.Turn} {GrammarDictionary.ChoiceOnOff.On}");
        public bool SwitchOffExists =>
            _text.Contains($"{GrammarDictionary.SwitchTurn.Switch} {GrammarDictionary.ChoiceOnOff.Off}")
            ||
            _text.Contains($"{GrammarDictionary.SwitchTurn.Turn} {GrammarDictionary.ChoiceOnOff.Off}");
    }
}