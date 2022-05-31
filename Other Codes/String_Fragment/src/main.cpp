#include <Arduino.h>

#define Fragment_Frame_Len 10

String str1 = "MAGENTA GROUP PRIVATE LIMITED";
char temp_str[Fragment_Frame_Len];

void setup()
{
  // put your setup code here, to run once:
  uint8_t i = 0;
  Serial.begin(115200);
  uint16_t total_len = str1.length();
  uint8_t number_of_frag = total_len / Fragment_Frame_Len;
  uint8_t remaining_data = total_len % Fragment_Frame_Len;
  Serial.printf("\nTotal Length: %d\nNo. of fragments: %d\nRemaining Data: %d\n", total_len, number_of_frag, remaining_data);
  for (i = 0; i < number_of_frag; i++)
  {
    for (uint8_t j = 0; j < Fragment_Frame_Len; j++)
      temp_str[j] = (char)str1[(Fragment_Frame_Len * i) + j];
    Serial.printf("Fragmented Frame %d: %s\tLength: %d\n", i + 1, temp_str, sizeof(temp_str));
    for (uint8_t j = 0; j < Fragment_Frame_Len; j++)
      temp_str[j] = ' ';
  }
  if (remaining_data > 0)
  {
    for (uint8_t j = 0; j < remaining_data; j++)
      temp_str[j] = (char)str1[(Fragment_Frame_Len * i) + j];
    Serial.printf("Fragmented Frame %d: %s\tLength: %d\n", i + 1, temp_str, sizeof(temp_str));
    for (uint8_t j = 0; j < Fragment_Frame_Len; j++)
      temp_str[j] = ' ';
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}