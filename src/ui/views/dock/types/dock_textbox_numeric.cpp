#include "./dock_textbox_numeric.h"

Component textboxWithLabel2 = wrapWithLabel(textbox, 0.02f);

Component createDockTextboxNumeric(DockTextboxNumeric& dockTextboxNumeric){
   static TextData textboxConfigData {
     .valueText = "",
     .cursorLocation = 0,
     .highlightLength = 0,
     .maxchars = -1,
   };
   std::function<void(TextData, int)> onEdit = [&dockTextboxNumeric](TextData textData, int rawKey) -> void {
     textboxConfigData = textData;
     float number;
     auto valid = maybeParseFloat(textData.valueText, number);
     if (valid){
       dockTextboxNumeric.onEdit(number, textData.valueText);
     }else {
      if (textData.valueText == "" || textData.valueText == "-"){
        dockTextboxNumeric.onEdit(0.f, textData.valueText);
      }
     }
   };
   TextData textData {
     .valueText = dockTextboxNumeric.value(),
     .cursorLocation = textboxConfigData.cursorLocation,
     .highlightLength = textboxConfigData.highlightLength,
     .maxchars = textboxConfigData.maxchars,
   };
   Props textboxProps {
     .props = {
       PropPair { .symbol = editableSymbol, .value = true },
       PropPair { .symbol = textDataSymbol, .value = textData },
       PropPair { .symbol = onInputSymbol, .value = onEdit },
       PropPair { .symbol = valueSymbol, .value = dockTextboxNumeric.label },
     }
   };
   auto textboxWithProps = withPropsCopy(textboxWithLabel2, textboxProps);
   return textboxWithProps;
}