#define LEAP_YEAR(Y) ((Y > 0) && !(Y % 4) && ((Y % 100) || !(Y % 400)))

String getFormattedDateTime(unsigned long secs);
String getFormattedTime(unsigned long rawTime);
String getFormattedDate(unsigned long secs);