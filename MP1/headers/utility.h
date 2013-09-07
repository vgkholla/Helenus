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

};

#endif