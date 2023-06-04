/* Example code from link below
https://hackaday.com/2012/07/16/automatic-daylight-savings-time-compensation-for-your-clock-projects/
*/

#include <arduino.h>

//int myYear = 2021;
char myDOW = 0; // Sunday
char myNthWeekBegin = 2; // second Sunday
char myMonthBegin = 3; // in March
char myNthWeekEnd = 1; // first Sunday
char myMonthEnd = 11; // in November

/*--------------------------------------------------------------------------
  FUNC: 6/11/11 - Returns day of week for any given date
  PARAMS: year, month, date
  RETURNS: day of week (0-7 is Sun-Sat)
  NOTES: Sakamoto's Algorithm
    http://en.wikipedia.org/wiki/Calculating_the_day_of_the_week#Sakamoto.27s_algorithm
    Altered to use char when possible to save microcontroller ram
--------------------------------------------------------------------------*/
char dow(int y, char m, char d)
{
    static char t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    y -= m < 3;
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

/*--------------------------------------------------------------------------
  FUNC: 6/11/11 - Returns the date for Nth day of month. For instance,
    it will return the numeric date for the 2nd Sunday of April
  PARAMS: year, month, day of week, Nth occurence of that day in that month
  RETURNS: date
  NOTES: There is no error checking for invalid inputs.
--------------------------------------------------------------------------*/
int NthDate(int year, char month, char DOW, char NthWeek)
{
    int targetDate = 1;
    int firstDOW = dow(year, month, targetDate);
    while (firstDOW != DOW) {
        firstDOW = (firstDOW + 1) % 7;
        targetDate++;
    }
    //Adjust for weeks
    targetDate += (NthWeek - 1) * 7;
    return targetDate;
}

int isDST(time_t UNIXTime)
{
    struct tm tmnow;
    gmtime_r(&UNIXTime, &tmnow);

    tmnow.tm_mon++; // Jan = 1
    tmnow.tm_year += 1900;

    if (tmnow.tm_mon > myMonthBegin && tmnow.tm_mon < myMonthEnd) { // if between March and November
        return 1;
    }

    if (tmnow.tm_mon == myMonthBegin) { // if March
        int DSTbegin = NthDate(tmnow.tm_year, myMonthBegin, myDOW, myNthWeekBegin);
        if (tmnow.tm_mday >= DSTbegin && tmnow.tm_hour >= 2)
            return 1;
    }

    if (tmnow.tm_mon == myMonthEnd) { // if November
        int DSTend = NthDate(tmnow.tm_year, myMonthEnd, myDOW, myNthWeekEnd);
        if (tmnow.tm_mday <= DSTend && tmnow.tm_hour < 2)
            return 1;
    }

    return 0;
}
