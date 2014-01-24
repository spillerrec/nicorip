/*
	This file is part of nicorip.

	nicorip is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	nicorip is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with nicorip.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QFile>
#include <QDebug>

#include <random>
#include <chrono>
using namespace std;


string generate_uid(){
	default_random_engine generator( chrono::system_clock::now().time_since_epoch().count() );
	uniform_int_distribution<char> distribution( '0', '9' );
	string uid;
	for( unsigned i=0; i<19; i++ )
		uid += distribution( generator );
	return uid;
}

void write_time( QIODevice& out, long double seconds ){
	int minutes = floor( seconds / 60 );
	seconds -= minutes * 60;
	int hours = minutes / 60;
	minutes -= hours * 60;
	QString time( "%1:%2:%3" );
	QString secs = QString::number( seconds, 'f', 9 );
	out.write( time
		.arg( hours, 2, 10, QChar('0') )
		.arg( minutes, 2, 10, QChar('0') )
		.arg( seconds < 10 ? "0" + secs : secs ).toLatin1()
		);
}

void write_chapter( QIODevice& out, const char* name, double begin, double end ){
	out.write( "<ChapterAtom><ChapterDisplay><ChapterString>" );
	out.write( name );
	out.write( "</ChapterString><ChapterLanguage>eng</ChapterLanguage></ChapterDisplay><ChapterUID>" );
	out.write( generate_uid().c_str() );
	out.write( "</ChapterUID><ChapterFlagHidden>0</ChapterFlagHidden><ChapterFlagEnabled>1</ChapterFlagEnabled><ChapterTimeStart>" );
	write_time( out, begin );
	out.write( "</ChapterTimeStart><ChapterTimeEnd>" );
	write_time( out, end );
	out.write( "</ChapterTimeEnd></ChapterAtom>" );
}

void create_chapters( const char* name, long double start, long double end, long double stop, unsigned amount=100 ){
	QFile out( "chapters_out.xml" );
	if( !out.open( QIODevice::WriteOnly ) )
		qFatal( "Output file could not be opened!" );
	
	out.write( "<?xml version=\"1.0\"?>\n<Chapters><EditionEntry><EditionFlagHidden>0</EditionFlagHidden><EditionFlagDefault>0</EditionFlagDefault><EditionUID>" );
	out.write( generate_uid().c_str() );
	out.write( "</EditionUID>" );
	
	out.write( "<ChapterAtom><ChapterDisplay><ChapterString>" );
	out.write( name );
	out.write( "</ChapterString><ChapterLanguage>eng</ChapterLanguage></ChapterDisplay><ChapterUID>" );
	out.write( generate_uid().c_str() );
	out.write( "</ChapterUID><ChapterFlagHidden>0</ChapterFlagHidden><ChapterFlagEnabled>1</ChapterFlagEnabled>" );
	
	write_chapter( out, "BEGIN", 0, start );
	
	for( unsigned i=1; i<=amount; i++ )
		write_chapter( out, QString::number(i).toLatin1().constData(), start, end );
	
	write_chapter( out, "END", end, stop );
	
	out.write( "<ChapterTimeStart>00:00:00.000000000</ChapterTimeStart><ChapterTimeEnd>" );
	write_time( out, start + (end-start)*amount + (stop-end) );
	out.write( "</ChapterTimeEnd></ChapterAtom><EditionFlagOrdered>1</EditionFlagOrdered></EditionEntry></Chapters>" );
}
