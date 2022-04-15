#include "SPITransfer.h"


SPITransfer myTransfer;

const int fileSize = 2000;
char file[fileSize] = "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Aenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. Donec pede justo, fringilla vel, aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo. Nullam dictum felis eu pede mollis pretium. Integer tincidunt. Cras dapibus. Vivamus elementum semper nisi. Aenean vulputate eleifend tellus. Aenean leo ligula, porttitor eu, consequat vitae, eleifend ac, enim. Aliquam lorem ante, dapibus in, viverra quis, feugiat a, tellus. Phasellus viverra nulla ut metus varius laoreet. Quisque rutrum. Aenean imperdiet. Etiam ultricies nisi vel augue. Curabitur ullamcorper ultricies nisi. Nam eget dui. Etiam rhoncus. Maecenas tempus, tellus eget condimentum rhoncus, sem quam semper libero, sit amet adipiscing sem neque sed ipsum. Nam quam nunc, blandit vel, luctus pulvinar, hendrerit id, lorem. Maecenas nec odio et ante tincidunt tempus. Donec vitae sapien ut libero venenatis faucibus. Nullam quis ante. Etiam sit amet orci eget eros faucibus tincidunt. Duis leo. Sed fringilla mauris sit amet nibh. Donec sodales sagittis magna. Sed consequat, leo eget bibendum sodales, augue velit cursus nunc, quis gravida magna mi a libero. Fusce vulputate eleifend sapien. Vestibulum purus quam, scelerisque ut, mollis sed, nonummy id, metus. Nullam accumsan lorem in dui. Cras ultricies mi eu turpis hendrerit fringilla. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; In ac dui quis mi consectetuer lacinia. Nam pretium turpis et arcu. Duis arcu tortor, suscipit eget, imperdiet nec, imperdiet iaculis, ipsum. Sed aliquam ultrices mauris. Integer ante arcu, accumsan a, consectetuer eget, posuere ut, mauris. Praesent adipiscing. Phasellus ullamcorper ipsum rutrum nunc. Nunc nonummy metus. Vestib";
char fileName[] = "test.txt";


void setup()
{
  Serial.begin(115200);
  
  digitalWrite(SS, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);

  myTransfer.begin(SPI);
}


void loop()
{
  myTransfer.sendDatum(fileName); // Send filename
  
  uint16_t numPackets = fileSize / (MAX_PACKET_SIZE - 2); // Reserve two bytes for current file index
  
  if (fileSize % MAX_PACKET_SIZE) // Add an extra transmission if needed
    numPackets++;
  
  for (uint16_t i=0; i<numPackets; i++) // Send all data within the file across multiple packets
  {
    uint16_t fileIndex = i * MAX_PACKET_SIZE; // Determine the current file index
    uint8_t dataLen = MAX_PACKET_SIZE - 2;

    if ((fileIndex + (MAX_PACKET_SIZE - 2)) > fileSize) // Determine data length for the last packet if file length is not an exact multiple of MAX_PACKET_SIZE-2
      dataLen = fileSize - fileIndex;
    
    uint8_t sendSize = myTransfer.txObj(fileIndex); // Stuff the current file index
    sendSize = myTransfer.txObj(file[fileIndex], sendSize, dataLen); // Stuff the current file data
    
    myTransfer.sendData(sendSize, 1); // Send the current file index and data
    delay(100);
  }
  delay(10000);
}
