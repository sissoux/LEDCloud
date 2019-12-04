
#include <FastLED.h>

char fullstrBuffer[64];

const char *getPort(void *portPtr) {
// AVR port checks
#ifdef PORTA
	if(portPtr == (void*)&PORTA) { return "PORTA"; }
#endif
#ifdef PORTB
	if(portPtr == (void*)&PORTB) { return "PORTB"; }
#endif
#ifdef PORTC
	if(portPtr == (void*)&PORTC) { return "PORTC"; }
#endif
#ifdef PORTD
	if(portPtr == (void*)&PORTD) { return "PORTD"; }
#endif
#ifdef PORTE
	if(portPtr == (void*)&PORTE) { return "PORTE"; }
#endif
#ifdef PORTF
	if(portPtr == (void*)&PORTF) { return "PORTF"; }
#endif
#ifdef PORTG
	if(portPtr == (void*)&PORTG) { return "PORTG"; }
#endif
#ifdef PORTH
	if(portPtr == (void*)&PORTH) { return "PORTH"; }
#endif
#ifdef PORTI
	if(portPtr == (void*)&PORTI) { return "PORTI"; }
#endif
#ifdef PORTJ
	if(portPtr == (void*)&PORTJ) { return "PORTJ"; }
#endif
#ifdef PORTK
	if(portPtr == (void*)&PORTK) { return "PORTK"; }
#endif
#ifdef PORTL
	if(portPtr == (void*)&PORTL) { return "PORTL"; }
#endif

// Teensy 3.x port checks
#ifdef GPIO_A_PDOR
	if(portPtr == (void*)&GPIO_A_PDOR) { return "GPIO_A_PDOR"; }
#endif
#ifdef GPIO_B_PDOR
	if(portPtr == (void*)&GPIO_B_PDOR) { return "GPIO_B_PDOR"; }
#endif
#ifdef GPIO_C_PDOR
	if(portPtr == (void*)&GPIO_C_PDOR) { return "GPIO_C_PDOR"; }
#endif
#ifdef GPIO_D_PDOR
	if(portPtr == (void*)&GPIO_D_PDOR) { return "GPIO_D_PDOR"; }
#endif
#ifdef GPIO_E_PDOR
	if(portPtr == (void*)&GPIO_E_PDOR) { return "GPIO_E_PDOR"; }
#endif
#ifdef REG_PIO_A_ODSR
	if(portPtr == (void*)&REG_PIO_A_ODSR) { return "REG_PIO_A_ODSR"; }
#endif
#ifdef REG_PIO_B_ODSR
	if(portPtr == (void*)&REG_PIO_B_ODSR) { return "REG_PIO_B_ODSR"; }
#endif
#ifdef REG_PIO_C_ODSR
	if(portPtr == (void*)&REG_PIO_C_ODSR) { return "REG_PIO_C_ODSR"; }
#endif
#ifdef REG_PIO_D_ODSR
	if(portPtr == (void*)&REG_PIO_D_ODSR) { return "REG_PIO_D_ODSR"; }
#endif

// Teensy 4 port checks
#ifdef GPIO1_DR
	if(portPtr == (void*)&GPIO1_DR) { return "GPIO1_DR"; }
#endif
#ifdef GPIO2_DR
if(portPtr == (void*)&GPIO2_DR) { return "GPIO21_DR"; }
#endif
#ifdef GPIO3_DR
if(portPtr == (void*)&GPIO3_DR) { return "GPIO3_DR"; }
#endif
#ifdef GPIO4_DR
if(portPtr == (void*)&GPIO4_DR) { return "GPIO4_DR"; }
#endif
  String unknown_str = "Unknown: " + String((size_t)portPtr, HEX);
	strncpy(fullstrBuffer, unknown_str.c_str(), unknown_str.length());
	fullstrBuffer[sizeof(fullstrBuffer)-1] = '\0';
	return fullstrBuffer;
}

template<uint8_t PIN> void CheckPin()
{
	CheckPin<PIN - 1>();

	RwReg *systemThinksPortIs = portOutputRegister(digitalPinToPort(PIN));
	RwReg systemThinksMaskIs = digitalPinToBitMask(PIN);

	Serial.print("Pin "); Serial.print(PIN); Serial.print(": Port ");

	if(systemThinksPortIs == FastPin<PIN>::port()) {
		Serial.print("valid & mask ");
	} else {
		Serial.print("invalid, is "); Serial.print(getPort((void*)FastPin<PIN>::port())); Serial.print(" should be ");
		Serial.print(getPort((void*)systemThinksPortIs));
		Serial.print(" & mask ");
	}

	if(systemThinksMaskIs == FastPin<PIN>::mask()) {
		Serial.println("valid.");
	} else {
		Serial.print("invalid, is "); Serial.print(FastPin<PIN>::mask()); Serial.print(" should be "); Serial.println(systemThinksMaskIs);
	}
}

template<> void CheckPin<-1> () {}

void setup() {
	delay(5000);
    Serial.begin(38400);
    Serial.println("resetting!");
}

void loop() {
	CheckPin<MAX_PIN>();
	delay(100000);

	Serial.print("GPIO_1_DR is: "); Serial.print(getPort((void*)&(GPIO1_DR)));
}
