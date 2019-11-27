
int adc_key_val[5] = { 50, 200, 400, 600, 800 };
int NUM_KEYS = 5;
int adc_key_in;
int key = -1;
int oldkey = -1;


void ReadButton()
{
  adc_key_in = analogRead(0);    // read the value from the sensor
  key = get_key(adc_key_in);  // convert into key press
  
  if (key != oldkey)   // if keypress is detected
  {
    delay(25);  // wait for debounce time
    adc_key_in = analogRead(0);    // read the value from the sensor
    key = get_key(adc_key_in);    // convert into key press
    if (key != oldkey)
    {
      oldkey = key;
      if (key != -1)
        ProcessKey(key);  
    }
  }
  delay(25);
}



// Convert ADC value to key number
int get_key(unsigned int input)
{
  int k;
  for (k = 0; k < NUM_KEYS; k++)
  {
    if (input < adc_key_val[k])
      return k;
  }
  
  if (k >= NUM_KEYS)
    k = -1;  // No valid key pressed
  
  return k;
}



void ProcessKey(int key)
{
  // Switch By Key Input
  switch (key)
  {
    case 3:
      ProcessLR(-1); break;
    case 0:
      ProcessLR(1); break;
    case 1:
      ProcessUD(1); break;
    case 2:
      ProcessUD(-1); break;
    case 4:
      if (itemSelect == 0)
        Fire();
      else if (itemSelect == 9)
        SaveOrLoad();
      else if (itemSelect > 2)
        ScaleChange(10);
      break;
  }
  
  // Clear Previous Info
  lcd.setCursor(0, 1);
  lcd.print("                ");
  
  // Switch By Menu Item
  lcd.setCursor(0, 1);
  switch (itemSelect)
  {
    case 0:
      if (standby)
        lcd.print("Ready           ");
      else
        lcd.print("Firing ......   ");
      break;
    case 1:
      lcd.print(*chanSet); break;
    case 2:
      lcd.print(chanCombo[*comboSelect]); break;
    case 9:
      lcd.print(saveLoad[*saveLoadSelect]); break;
    default:
      lcd.print(paraMatrix[itemSelect][*chanSet]);
      lcd.print(" ");
      lcd.print(units[itemSelect]);  
      lcd.setCursor(10, 1);
      lcd.print(numChangeFactor);
      lcd.print("X     ");
  }
}



void ProcessLR(int dir)
{
  if (dir < 0)
  {
    if(itemSelect != 0)
      itemSelect--;
    else
      itemSelect = 9;
  }
  else
  {
    if(itemSelect != 9)
      itemSelect++;
    else
      itemSelect = 0;
  }
  
  lcd.setCursor(0, 0);
  lcd.print(msgs[itemSelect]);
}



void ProcessUD(int dir)
{
  if (itemSelect == 1)
    ChannelChange(dir);
  else if (itemSelect == 2)
    ComboChange(dir);
  else if (itemSelect == 9)
    SaveLoadChange();
  else if (itemSelect > 2)
    NumChange(itemSelect, *chanSet, dir);
  
  lcd.setCursor(0, 0);
  lcd.print(msgs[itemSelect]);
}



void ScaleChange(byte factor)
{
  if (numChangeFactor == 10000)
    numChangeFactor = 1;
  else
    numChangeFactor *= factor;
  
  lcd.setCursor(10, 1);
  lcd.print(numChangeFactor);
  lcd.print("X     ");
}



void NumChange(byte item, byte chan, int dir)
{
  unsigned int val = paraMatrix[item][chan];
  
  if (dir > 0)
  {
    val += numChangeFactor;
    if (val > paraMatrix[item][chan])
      paraMatrix[item][chan] = val;
  }
  else
  {
    val -= numChangeFactor;
    if (val < paraMatrix[item][chan])
      paraMatrix[item][chan] = val;
  }
}


void ChannelChange(int dir)
{
  if (dir < 0 & *chanSet > 0)
    *chanSet += dir;
  else if (dir > 0 & *chanSet < 2)
    *chanSet += dir;
}


void ComboChange(int dir)
{
  if (dir < 0 & *comboSelect > 1)
    *comboSelect += dir;
  else if (dir > 0 & *comboSelect < 7)
    *comboSelect += dir;
}


void SaveLoadChange()
{
  *saveLoadSelect = !*saveLoadSelect;
}




