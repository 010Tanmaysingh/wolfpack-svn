//  boost progress.hpp header file  ------------------------------------------//

//  (C) Copyright Beman Dawes 1994-99. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version including documentation.

//  Revision History
//  20 May 01  Introduce several static_casts<> to eliminate warning messages
//             (Fixed by Beman, reported by Herve Bronnimann)
//  12 Jan 01  Change to inline implementation to allow use without library
//             builds. See docs for more rationale. (Beman Dawes) 
//  22 Jul 99  Name changed to .hpp
//  16 Jul 99  Second beta
//   6 Jul 99  Initial boost version

#if !defined(__PROGRESS_H__)
#define __PROGRESS_H__

//#include <iostream>
#include "wpconsole.h"

extern WPConsole_cl clConsole;

class progress_display
{
public:
	explicit progress_display( unsigned long expected_count )
	// os is hint; implementation may ignore
	{ restart(expected_count); }

	void restart( unsigned long expected_count )
	//  Effects: display appropriate scale
	//  Postconditions: count()==0, expected_count()==expected_count
	{
		_count = _next_tic_count = _tic = 0;
		_expected_count = expected_count;

		clConsole.send("\n0%   10   20   30   40   50   60   70   80   90   100%\n");
		clConsole.send("|----|----|----|----|----|----|----|----|----|----|\n");
		if ( !_expected_count ) 
		_expected_count = 1;  // prevent divide by zero
	} // restart

	unsigned long  operator+=( unsigned long increment )
	//  Effects: Display appropriate progress tic if needed.
	//  Postconditions: count()== original count() + increment
	//  Returns: count().
	{
		if ( (_count += increment) >= _next_tic_count ) { display_tic(); }
			return _count;
	}

	unsigned long  operator++()           { return operator+=( 1 ); }
	unsigned long  count() const          { return _count; }
	unsigned long  expected_count() const { return _expected_count; }

private:
//	std::ostream & _os; // may not be present in all imps
	unsigned long _count, _expected_count, _next_tic_count;
	unsigned int  _tic;
	void display_tic()
	{
		// use of floating point ensures that both large and small counts
	    // work correctly.  static_cast<>() is also used several places
		// to suppress spurious compiler warnings. 
		unsigned int tics_needed = 	static_cast<unsigned int>( (static_cast<double>(_count)/_expected_count)*50.0 );
		do 
		{ 
			clConsole.send("*");
		} 
		while ( ++_tic < tics_needed );
		_next_tic_count = static_cast<unsigned long>((_tic/50.0)*_expected_count);
		if ( _count == _expected_count ) {
			if ( _tic < 51 ) 
				clConsole.send("*\n");
			else if( _tic == 51 )
				clConsole.send("\n");
		}
	} // display_tic
};

#endif //__PROGRESS_H__
