
int incomingVal = 0;
String incomingCmd = String();

void ReadSerialCommand()
{
  if (Serial.available() > 0)
  {
    char b = Serial.read();
    if (isDigit(b))
    {
      incomingVal = incomingVal*10  + b - '0';
    }
    else if (islower(b))
    {
      switch(b)
      {
        case 'i':
          paraMatrix[3][*chanSet] = incomingVal;
          Serial.print("Pulse interval is ");
          Serial.print(incomingVal);
          Serial.println(" ms");
          break;
        case 'm':
          paraMatrix[4][*chanSet] = incomingVal;
          Serial.print("Number of pulses in a train is ");
          Serial.println(incomingVal);
          break;
        case 'w':
          paraMatrix[5][*chanSet] = incomingVal;
          Serial.print("Pulse duration is ");
          Serial.print(incomingVal);
          Serial.println(" ms");
          break;
        case 'r':
          paraMatrix[6][*chanSet] = incomingVal;
          Serial.print("Repeat number of the train is ");
          Serial.println(incomingVal);
          break;
        case 'v':
          paraMatrix[7][*chanSet] = incomingVal;
          Serial.print("Inter-train interval is ");
          Serial.print(incomingVal);
          Serial.println(" ms");
          break;
        case 'p':
          Fire();
          break;
        case 's':
          Serial.println("Aborted");
          break;
        default:
          break;
      }
      incomingVal = 0;
    }
    else if (isalpha(b))
    {
      incomingCmd += b;
      if (incomingCmd.length() == 3)
      {
        if(incomingCmd.equals("MPZ"))
        {
          Serial.println("YES");
        }
        incomingCmd = "";
      }
    }
  } 
}
