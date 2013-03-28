#include <Servo.h>
#include "Wire.h"
#define DS1307_I2C_ADDRESS 0x68  // This is the I2C address

// Arduino version compatibility Pre-Compiler Directives
#if defined(ARDUINO) && ARDUINO >= 100   // Arduino v1.0 and newer
  #define I2C_WRITE Wire.write 
  #define I2C_READ Wire.read
#else                                   // Arduino Prior to v1.0 
  #define I2C_WRITE Wire.send 
  #define I2C_READ Wire.receive
#endif

// TODO: Use these where possible.
#define PANEL_SERVO_OUT_PIN 9
#define PANEL_SERVO_IN_PIN 0
#define LOCK_SERVO_OUT_PIN 10
#define BELL_OUT_PIN 13
#define CLOCK_SCL_PIN 0
#define CLOCK_SDA_PIN 0

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
	return ( (val/10*16) + (val%10) );
}
 
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
	return ( (val/16*10) + (val%16) );
}

/******************************************************************************
 * Date Class
 ******************************************************************************
 * This class contains the information needed to encapsulate a date object.
 * Notable members include dayOfWeek, dayOfMonth, month, year.
 * These members are redundant, and should be checked and maintained by
 * structural invariance.
 ******************************************************************************/
 
char *Day[] = {"","Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
char  *Mon[] = {"","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

class Date {
private:
	byte dayOfWeek, dayOfMonth, month, year;
public:
	Date() {}
	Date( byte DayOfWeek, byte DayOfMonth, byte Month, byte Year ) {
		//TODO: Ensure values are valid and consistent.
		dayOfWeek = DayOfWeek;
		dayOfMonth = DayOfMonth;
		month = Month;
		year = Year;
	}
	
	byte getDayOfWeek() {
		return dayOfWeek;
	}
	
	byte getDayOfMonth() {
		return dayOfMonth;
	}
	
	byte getMonth() {
		return month;
	}
	
	byte getYear() {
		return year;
	}
	
	void printToSerial() {
		Serial.print(Day[getDayOfWeek()]);
		Serial.print(", ");
		Serial.print(getDayOfMonth(), DEC);
		Serial.print(" ");
		Serial.print(Mon[getMonth()]);
		Serial.print(" 20");
		if (getYear() < 10)
			Serial.print("0");
		Serial.print(getYear(), DEC);
	}
};

/******************************************************************************
 * Time Class
 ******************************************************************************
 * This class contains the information needed to encapsulate a time object.
 * The time is rounded to the nearest second. It is assumed no further
 * granularity is useful. This class may be used to represent either a time
 * interval, or a time with respect to midnight.
 ******************************************************************************/
class Time {
private:
	byte second, minute, hour;
public:
	Time() {}
	Time( byte Second, byte Minute, byte Hour ) {
		//TODO: Ensure validity.
		second = Second;
		minute = Minute;
		hour = Hour;
	}
	
	byte getSecond() {
		return second;
	}
	
	byte getMinute() {
		return minute;
	}
	
	byte getHour() {
		return hour;
	}
	
	void printToSerial() {
		if (getHour() < 10)
			Serial.print("0");
		Serial.print(getHour(), DEC);
		Serial.print(":");
		if (getMinute() < 10)
			Serial.print("0");
		Serial.print(getMinute(), DEC);
		Serial.print(":");
		if (getSecond() < 10)
			Serial.print("0");
		Serial.print(getSecond(), DEC);
	}

	boolean operator == (Time &other) {
  		return ( this->getHour() == other.getHour() ) && ( this->getMinute() == other.getMinute() ) && ( this->getSecond() == other.getSecond() );
	}

	boolean operator > (Time &other) {
  		if( this->getHour() == other.getHour() ) {
	  		if( this->getMinute() == other.getMinute() ) {
				return this->getSecond() > other.getSecond();
			}
			return this->getMinute() > other.getMinute();
		}
		return this->getHour() > other.getHour();
	}

	boolean operator < (Time &other) {
  		if( this->getHour() == other.getHour() ) {
	  		if( this->getMinute() == other.getMinute() ) {
				return this->getSecond() < other.getSecond();
			}
			return this->getMinute() < other.getMinute();
		}
		return this->getHour() < other.getHour();
	}
	
	static boolean isInRange( Time startTime, Time endTime, Time time ) {
		if( startTime == endTime ) {
			return false;
		}
		if( startTime < endTime ) {
			return ( ( startTime < time ) && ( time < endTime ) );
		}
		// The time interval includes midnight. It wraps around.
		return !( Time::isInRange( endTime, startTime, time ) );
	}
};

/******************************************************************************
 * DateTime Class
 ******************************************************************************
 * Combines Date and Time to represent a time on a particular date.
 ******************************************************************************/
class DateTime {
private:
	Date date;
	Time time;
public:
	DateTime( byte Second, byte Minute, byte Hour, byte DayOfWeek, byte DayOfMonth, byte Month, byte Year ) {
		date = Date( DayOfWeek, DayOfMonth, Month, Year );
		time = Time( Second, Minute, Hour );
	}
	
	Time getTime() {
		return time;
	}
	
	Date getDate() {
  		return date;
	}
};

/******************************************************************************
 * Clock Class
 ******************************************************************************
 * Encapsulates the DS1307 Clock Module.
 ******************************************************************************/
class Clock {
private:
	// Nothing Private for now. The module itself has the required memory.
public:
	void setDateTime( DateTime dt ) {
		
		Wire.beginTransmission(DS1307_I2C_ADDRESS);
		I2C_WRITE(0x00);
	
		I2C_WRITE(decToBcd( dt.getTime().getSecond() ) & 0x7f);	// 0 to bit 7 starts the clock
		I2C_WRITE(decToBcd( dt.getTime().getMinute() ));
		I2C_WRITE(decToBcd( dt.getTime().getHour() ));	// If you want 12 hour am/pm you need to set
						// bit 6 (also need to change readDateDs1307)
		I2C_WRITE(decToBcd( dt.getDate().getDayOfWeek() ));
		I2C_WRITE(decToBcd( dt.getDate().getDayOfMonth() ));
		I2C_WRITE(decToBcd( dt.getDate().getMonth() ));
		I2C_WRITE(decToBcd( dt.getDate().getYear() ));
		Wire.endTransmission();
	}

	DateTime getDateTime() {
		// Reset the register pointer
		Wire.beginTransmission(DS1307_I2C_ADDRESS);
		I2C_WRITE(0x00);
		Wire.endTransmission();

		Wire.requestFrom(DS1307_I2C_ADDRESS, 7);

		// A few of these need masks because certain bits are control bits
		byte second     = bcdToDec(I2C_READ() & 0x7f);
		byte minute     = bcdToDec(I2C_READ());
		
		// Hours are in 24 hour time.
		byte hour       = bcdToDec(I2C_READ() & 0x3f);
		byte dayOfWeek  = bcdToDec(I2C_READ());
		byte dayOfMonth = bcdToDec(I2C_READ());
		byte month      = bcdToDec(I2C_READ());
		byte year       = bcdToDec(I2C_READ());
	
		return DateTime( second, minute, hour, dayOfWeek, dayOfMonth, month, year );
	}
	
	static DateTime serialReadDateTime() {
		byte second = (byte) ((Serial.read() - 48) * 10 + (Serial.read() - 48));
		byte minute = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
		byte hour  = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
		byte dayOfWeek = (byte) (Serial.read() - 48);
		byte dayOfMonth = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
		byte month = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
		byte year= (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
		
		return DateTime( second, minute, hour, dayOfWeek, dayOfMonth, month, year );
	}
};

/******************************************************************************
 * ServoPositionEstimator Class
 ******************************************************************************
 * Abstract class to define an interface of an estimator of the servo position.
 ******************************************************************************/
class ServoPositionEstimator {
public:
	// Returns the current estimation of the servo position.
	virtual int getPosition() = 0;
	// Artificially add an estimated position. It's a good
	// idea to call this after forcing a position.
	virtual void setPosition( int pose ) = 0;
};

/******************************************************************************
 * VirtualServoPositionEstimator Class
 ******************************************************************************
 * A dummy class to allow for general instantiation. Just tracks the last given
 * value.
 ******************************************************************************/
class VirtualServoPositionEstimator : public ServoPositionEstimator {
private:
	int estimatedPose;
public:
	VirtualServoPositionEstimator() {
		estimatedPose = 0;
	}
	int getPosition() {
		return estimatedPose;
	}
	void setPosition( int pose ) {
		estimatedPose = pose;
	}
};

/******************************************************************************
 * PhysicalServoPositionEstimator Class
 ******************************************************************************
 * Actually attempt to read the value off the pin.
 ******************************************************************************/
class PhysicalServoPositionEstimator : public ServoPositionEstimator {
private:
	float estimatedPose;
	int pin;
	int readPosition() {
  		return 12 * analogRead( pin ) - 3111;
	}
public:
	PhysicalServoPositionEstimator() {
		pin = -1;
	}
	PhysicalServoPositionEstimator( int inputPin ) {
		pin = inputPin;
		estimatedPose = this->readPosition();
	}
	virtual int getPosition() {
		return (int)estimatedPose;
	}
	virtual void setPosition( int pose ) {
		// Totally ignore whatever we were just sent.
		// Load our physicallly based estimation, and if it
		// is not absurd, integrate it into our estimation.
		pose = this->readPosition();
		if( pose > 0 ) estimatedPose = .6 * estimatedPose + .4 * this->readPosition();
	}
};

/******************************************************************************
 * BistableServo Class
 ******************************************************************************
 * Defines a servo with two stable positions, and a simple interface to move
 * between them.
 ******************************************************************************/
class BistableServo {
  
private:
	int closed_position;
	int open_position;
	int current_position;

	int servo_pin;
	Servo servo;

	ServoPositionEstimator* positionEstimator;
  
	void moveto( int position ) {
		// Get our current position...
                current_position = positionEstimator->getPosition();
		servo.attach( servo_pin );
		while( current_position < position )
		{
			servo.write( current_position ); 
			delay(30);
			current_position++;
			positionEstimator->setPosition( current_position ); 
		}
		while( current_position > position )
		{
			servo.write( current_position );
			delay(30);
			current_position--;
			positionEstimator->setPosition( current_position ); 
		}
		servo.detach(); // Relax the servo and stop using power.
	}    
  
public:
	BistableServo() {
    
		closed_position = -1;
		open_position = -1;
		current_position = closed_position;

		servo_pin = -1;

		positionEstimator = NULL;
	}
	
	// Initialize Member Variables. Heavily Dependant on physical implementation.
	BistableServo( int output_pin, int closed_pos, int open_pos, ServoPositionEstimator* se ) {
		closed_position = closed_pos;
		open_position = open_pos;
		current_position = closed_position; // Start the servo closed.

		servo_pin = output_pin;

		positionEstimator = se;
		positionEstimator->setPosition( current_position );
	}
  
	// Moves the door to the open position. 
	void open() {
		moveto( open_position );
	}

	// Moves the door to the closed position.
	void close() {
		moveto( closed_position );
	}
  
	void hold() {
		servo.attach( servo_pin );
		servo.write( current_position );
		positionEstimator->setPosition( current_position );
	}
  
	void release() {
		servo.detach();
	}
  
};

/******************************************************************************
 * Door Class
 ******************************************************************************
 * Encapsulates all the hardware and software checks to make the door.
 * The door can do two things: open and close. The rest is handled by internal
 * code.
 ******************************************************************************/
class Door {
private:
	BistableServo panel;
	PhysicalServoPositionEstimator panelEstimator; // This assumes the panel Servo has been modified to provide position feedback.
	BistableServo lock;
	VirtualServoPositionEstimator lockEstimator; // The lock has no physical position estimation.
	boolean state;
public:
	Door() {
		//TODO: Replace all values with precompiler #defines.
		panelEstimator = PhysicalServoPositionEstimator( PANEL_SERVO_IN_PIN );
		panel = BistableServo( PANEL_SERVO_OUT_PIN, 0, 110, &panelEstimator );
		lockEstimator = VirtualServoPositionEstimator();
		lock = BistableServo( LOCK_SERVO_OUT_PIN, 95, 0, &lockEstimator );
	}
  
	void open() {
		lock.open();
		panel.open();
		state = false;
	}

	void ensureOpen() {
		for( int i = 0; i < 10; i++ ){ panelEstimator.setPosition(0); delay(100); } // Update our position estimation...
		if( panelEstimator.getPosition() < 50 && !isClosed() ) panel.open();
	}

	void close() {
		lock.open();
		delay( 1000 );
		panel.close();
		panel.hold();//TODO: Try without this.
		lock.close();
		panel.release();
		state = true;
	}
  
	bool isClosed() {
		return state;
	}
    
};

/******************************************************************************
 * Bell Class
 ******************************************************************************
 * Encapsulates the bell hardware, and simplifies making it ring.
 ******************************************************************************/
class Bell {
private:
	int pin;
public:
	Bell() {
		pin = BELL_OUT_PIN;

		pinMode(pin, OUTPUT);
		digitalWrite(pin, HIGH);
	}
  
	void ring( long time ) {
		digitalWrite( pin, LOW );
		time = time - 1000; // The last ring will take on average 1000ms to complete. Subtract off.

		if( time < 10 ) {
			time = 10;
		}
		delay( time );
		digitalWrite( pin, HIGH );
	}
};

Door door;
Bell bell;
Clock clock;

void setup() {
	Wire.begin();
	Serial.begin(57600); 
	door = Door();
	door.close();
	bell = Bell();
	clock = Clock();
} 

void loop()
{
	if ( Serial.available() ) {
		// Read a command off the serial port, and switch on it. 
		switch ( Serial.read() ) {
			
			// If command = "Tt" Set Date
			case 84:
			case 116:
				clock.setDateTime( Clock::serialReadDateTime() );
				Serial.println("Updated Date and Time.");
				Serial.print("Date: ");
				clock.getDateTime().getDate().printToSerial();
				Serial.print("\nTime: ");
				clock.getDateTime().getTime().printToSerial();
				Serial.print("\n");
				break;

			// If command = "Rr" Read Date ...
			case 82:
			case 114:
				Serial.println("The current data in memory is:");
				Serial.print("Date: ");
				clock.getDateTime().getDate().printToSerial();
				Serial.print("\nTime: ");
				clock.getDateTime().getTime().printToSerial();
				Serial.print("\n");
				break;

			// If command = "Qq" RTC1307 Memory Functions
			case 81:
			case 113:
				// Pause to make sure that we get the next command...
				delay(100);
				if (Serial.available()) {
					int i;
					switch ( Serial.read() ) {
						
						// If command = "1" RTC1307 Initialize Memory - All Data will be set to 255 (0xff).  Therefore 255 or 0 will be an invalid value.  
						case 49:
							Wire.beginTransmission(DS1307_I2C_ADDRESS); // 255 will be the init value and 0 will be considered an error that occurs when the RTC is in Battery mode.
							I2C_WRITE(0x08); // Set the register pointer to be just past the date/time registers.
							for (i = 1; i <= 24; i++) {
								I2C_WRITE(0Xff);
								delay(10);
							}   
							Wire.endTransmission();
							Wire.beginTransmission(DS1307_I2C_ADDRESS);   
							I2C_WRITE(0x21); // Set the register pointer to 33 for second half of registers. Only 32 writes per connection allowed.
							for (i = 1; i <= 33; i++) {
								I2C_WRITE(0Xff);
								delay(10);
							}   
							Wire.endTransmission();
							
							Serial.print("Date: ");
							clock.getDateTime().getDate().printToSerial();
							Serial.print("\nTime: ");
							clock.getDateTime().getTime().printToSerial();
							Serial.print("\n");
							
							Serial.println(": RTC1307 Initialized Memory");
							break;
						
						// If command = "2" RTC1307 Memory Dump
						case 50:
							Serial.print("Date: ");
							clock.getDateTime().getDate().printToSerial();
							Serial.print("\nTime: ");
							clock.getDateTime().getTime().printToSerial();
							Serial.print("\n");
							
							Serial.println(": RTC 1307 Dump Begin");
							Wire.beginTransmission(DS1307_I2C_ADDRESS);
							I2C_WRITE(0x00);
							Wire.endTransmission();
							Wire.requestFrom(DS1307_I2C_ADDRESS, 32);
							for (i = 0; i <= 31; i++) {  //Register 0-31 - only 32 registers allowed per I2C connection
								byte test = I2C_READ();
								Serial.print(i);
								Serial.print(": ");
								Serial.print(test, DEC);
								Serial.print(" : ");
								Serial.println(test, HEX);
							}
							Wire.beginTransmission(DS1307_I2C_ADDRESS);
							I2C_WRITE(0x20);
							Wire.endTransmission();
							Wire.requestFrom(DS1307_I2C_ADDRESS, 32);  
							for (i = 32; i <= 63; i++) {         //Register 32-63 - only 32 registers allowed per I2C connection
								byte test = I2C_READ();
								Serial.print(i);
								Serial.print(": ");
								Serial.print(test, DEC);
								Serial.print(" : ");
								Serial.println(test, HEX);
							}
							Serial.println(" RTC1307 Dump end");
							break;
					}
				}
				break;
		}
	}

	DateTime dt = clock.getDateTime();
	// TODO: Determine these times based on the rough times of sunset, sunrise...
	Time openTime = Time(0,30,8); // Open at 8:30

	Time closeTime; // Close at the appropriate time, based on the month.
	switch( dt.getDate().getMonth() ) {
		case 1:
			closeTime = Time(0,34,17);
			break;
		case 2:
			closeTime = Time(0,11,18);
			break;
		case 3:
			closeTime = Time(0,44,19);
			break;
		case 4:
			closeTime = Time(0,20,20);
			break;
		case 5:
			closeTime = Time(0,56,20);
			break;
		case 6:
			closeTime = Time(0,23,21);
			break;
		case 7:
			closeTime = Time(0,19,21);
			break;
		case 8:
			closeTime = Time(0,43,20);
			break;
		case 9:
			closeTime = Time(0,15,19);
			break;
		case 10:
			closeTime = Time(0,58,18);
			break;
		case 11:
			closeTime = Time(0,20,17);
			break;
		case 12:
			closeTime = Time(0,11,17);
			break;
	}

	// If the door is supposed to be open...
	if ( Time::isInRange( openTime, closeTime, dt.getTime() ) ) {
  		// If the door is closed despite this, open it.
  		if( door.isClosed() ) {
			Serial.println("Opening Door");
			door.open();
		}
		else {
			door.ensureOpen();
		}
	}

	// If the door is supposed to be closed...
	if ( Time::isInRange( closeTime, openTime, dt.getTime() ) ) {
  		// If the door is open despite this, close it.
  		if( !(door.isClosed()) ) {
  			Serial.println("Closing Door");
			bell.ring( 2000 ); // TODO: Change this to be more reasonable.
			delay( 2000 );
			door.close();
		}
		//TODO: Ensure it's locked?
	}
	delay( 500 );
}
