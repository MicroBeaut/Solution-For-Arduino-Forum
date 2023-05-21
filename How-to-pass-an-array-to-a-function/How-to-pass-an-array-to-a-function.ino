/*
  Solution For:
  Topic:      How to pass an array to a function?
  Category:   Programming Questions
  Link:       https://forum.arduino.cc/t/how-to-pass-an-array-to-a-function/1128553

  Date:       19-May-23
  Author:     MicroBeaut (μB)
*/

typedef struct {
  char number[14];
  bool contains(char *message) {
    return strstr(message, number) != nullptr;
  }
} PhoneNumber;

const PhoneNumber phoneNumber1 = {"+380951xxxxxx"};
const PhoneNumber phoneNumber2 = {"+380952xxxxxx"};
const PhoneNumber phoneNumber3 = {"+380953xxxxxx"};

PhoneNumber adminPhoneNumbers[] = {
  phoneNumber1,
  phoneNumber2,
  phoneNumber3
};
const size_t size = sizeof(adminPhoneNumbers) / sizeof(PhoneNumber);

char *message[] = {
  "dfgdfg dfg fdgf dfg dffg +380951xxxxxx fg dfg df ",
  "asdfjkl; asdf jkl; +380955xxxxxx asdf jkl;",
  "qweruiop qwer uiop +380953xxxxxx qwer uiop"
};
const uint8_t numberOfMessages = 3;

char *PhoneNumberInMessage(char *message, PhoneNumber *phoneNumbers, size_t count);

void setup() {
  Serial.begin(115200);
  for (size_t index = 0; index < numberOfMessages; index++) {
    char *phoneNumber = PhoneNumberInMessage(message[index], adminPhoneNumbers, size);
    Serial.print("Message: \"" + String(message[index]) + "\" , Contains the admin phone number:");
    phoneNumber ? Serial.println(phoneNumber) : Serial.println("-");
  }
  String ll;
  printf();
}

void loop() {

}

char *PhoneNumberInMessage(char *message, PhoneNumber *phoneNumbers, size_t count) {
  for (size_t index = 0; index < size; index++) {
    if (phoneNumbers[index].contains(message)) return phoneNumbers[index].number;
  }
  return nullptr;
}

//The sizeof operator using a pointer variable operand cannot be used as the number-of-elements expression in an array declaration.