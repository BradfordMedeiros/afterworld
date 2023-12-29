#include "./dock_textbox.h"

Component textboxWithLabel = wrapWithLabel(textbox, 0.02f);

Component createDockTextbox(DockTextboxConfig& textboxOptions){
   static TextData textboxConfigData {
     .valueText = "somedebugtext",
     .cursorLocation = 0,
     .highlightLength = 0,
     .maxchars = -1,
   };
   std::function<void(TextData, int)> onEdit = [&textboxOptions](TextData textData, int rawKey) -> void {
     textboxConfigData = textData;
     textboxOptions.onEdit(textData.valueText);
   };
   TextData textData {
     .valueText = textboxOptions.text(),
     .cursorLocation = textboxConfigData.cursorLocation,
     .highlightLength = textboxConfigData.highlightLength,
     .maxchars = textboxConfigData.maxchars,
   };
   Props textboxProps {
     .props = {
       PropPair { .symbol = editableSymbol, .value = true },
       PropPair { .symbol = textDataSymbol, .value = textData },
       PropPair { .symbol = onInputSymbol, .value = onEdit },
       PropPair { .symbol = valueSymbol, .value = textboxOptions.label },
     }
   };
   auto textboxWithProps = withPropsCopy(textboxWithLabel, textboxProps);
   return textboxWithProps;
}