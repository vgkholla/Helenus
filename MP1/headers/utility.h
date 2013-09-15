#ifndef AG_UTILITY
#define AG_UTILITY
//don't want multiple declarations

#include <iomanip>
#include <locale>
#include <sstream>
#include <string>

using namespace std;

class Utility {

	public:

	/**
	 * [utility function to convert an integer to a string]
	 * @param  num the integer to be converted
	 * @return the integer converted to string
	 */
	static string intToString(int num) {
		return static_cast<ostringstream*>( &(ostringstream() << num) )->str();
	} 

	/**
	 * [trims out trailing spaces]
	 * @param  str [input string]
	 * @return     [string without trailing spaces]
	 */
	static string trimTrailingSpaces(string str) {
		size_t strlength = str.length();
		while(str[strlength - 1] == ' ' || str[strlength - 1] == '\t') {
			str.erase(strlength - 1, strlength);
			strlength = str.length();
		}
		return str;
	}

	/**
	 * [check if character is escaped]
	 * @param  str [the string]
	 * @param  pos [the position of character]
	 * @return     [whether the character is escaped or not]
	 */
	static int isEscaped(string str, size_t pos) {
		int i = 0;
		while(str[pos - 1] == '\\'){
			i++;
			pos--;
		}

		return i % 2;
	}

};

#endif