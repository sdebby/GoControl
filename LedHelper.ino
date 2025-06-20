  /* Led helper
  Led colors
  r - red
  g - green
  b - blue
  y - yellow
  w - white
  p - pink
  a - amber
  o - black
  */

void RGBLed(char l3d){

  switch (l3d) {
    case 'r':
      rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0 ,0);   // Turn the RGB LED red
      break;
    case 'g':
      rgbLedWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS ,0);   // Turn the RGB LED green
      break;
    case 'b':
      rgbLedWrite(RGB_BUILTIN, 0, 0 ,RGB_BRIGHTNESS);   // Turn the RGB LED blue
      break;
    case 'y':
      rgbLedWrite(RGB_BUILTIN, 255, 255 ,0);   // Turn the RGB LED yellow
      break;
    case 'w':
      digitalWrite(RGB_BUILTIN, HIGH);   // Turn the RGB LED white
      break;
    case 'o':
      digitalWrite(RGB_BUILTIN, LOW);   // Turn the RGB LED white
      break;
    case 'p':
      rgbLedWrite(RGB_BUILTIN, 224, 33 ,38);   // Turn the RGB LED Pink
      break;
    case 'a':
      rgbLedWrite(RGB_BUILTIN, 255, 126 ,0);   // Turn the RGB LED Amber
      break;

    default:
      Serial.println("Select r, g, b, w, y,  o");
      break;
  }
}
