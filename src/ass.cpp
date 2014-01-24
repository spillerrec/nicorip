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

#include <QStringList>
#include <QFile>
#include <QTime>
#include <QDebug>

#include "pugixml/pugixml.hpp"
using namespace pugi;

#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
using namespace std;

struct Color{
	string name;
	unsigned color;
	Color( string name, unsigned color ) : name(name), color(color) { }
};

const Color white { "white", 0xFFFFFF };
const vector<Color> nico_colors{
		white
	,	{ "red",    0xFF0000 }
	,	{ "pink",   0xFF8080 }
	,	{ "orange", 0xFFC000 }
	,	{ "yellow", 0xFFFF00 }
	,	{ "green",  0x00FF00 }
	,	{ "cyan",   0x00FFFF }
	,	{ "blue",   0x0000FF }
	,	{ "purple", 0xC000FF }
	,	{ "black",  0x000000 }
	
	//TODO: premium colors, unknown color values
	,	{ "pink2",  0xFF8080 }
	};

class Comment{
	private: //Convenience methods
		template<typename T>
		static void check_and_set( QStringList& list, const char* name, T& output, T value ){
			if( list.contains( name ) ){
				output = value;
				list.removeAll( name );
			}
		}
		
	public:
		int time; //seconds * 100
		string contents;
		Color color{ white };
		
		enum Size{
			SMALL,
			NORMAL,
			BIG
		} size{ NORMAL };
		
		enum Position{
			ABOVE,
			MIDDLE,
			BELOW
		} pos{ MIDDLE };
		
	public:
		Comment( int time, string properties, string contents )
			:	time(time), contents(contents)
			{
				QString name = QString::fromUtf8( properties.c_str() );
				QStringList items = name.split( " ", QString::SkipEmptyParts );
				
				//Some number.. TODO: understand
				if( items.count() > 0 ){
					int number = items[0].toInt(); //TODO: check?
					items.removeAt( 0 );
				}
				
				//Size and position
				check_and_set( items, "big", size, BIG );
				check_and_set( items, "small", size, SMALL );
				check_and_set( items, "ue", pos, ABOVE );
				check_and_set( items, "shita", pos, BELOW );
				
				//Color
				for( auto c : nico_colors )
					check_and_set( items, c.name.c_str(), color, c );
				
				if( items.count() > 0 )
					qDebug() << "Unknown arguments: " << items.join( " " );
			}
		
		bool operator<( const Comment& other ) const{
			return time < other.time;
		}
};


class AssLine{
	private: //Convenience methods
		static QString time( double sec ){
			return QTime(0,0).addMSecs( sec*1000 ).toString( "hh:mm:ss.zzz" );
		}
		
		static void write( QIODevice& out, QString text ){
			out.write( text.toLatin1() ); //TODO: best conversion?
		}
		
	public:
		double start;
		double duration;
		unsigned size{ 28 };
		unsigned color{ white.color };
		int v_pos;
		Comment::Position pos{ Comment::MIDDLE };
		const char* content;
		
		AssLine( double start, double duration, int v_pos, const char* content )
			: start(start), duration(duration), v_pos(v_pos), content( content ) { }
		
		
		void write( QIODevice& out ){
			QString dialog( "Dialogue: 0,%1,%2,Default,,0,0,0,,{" );
			write( out, dialog.arg( time( start ), time( start+duration ) ) );
			
			//Position
			switch( pos ){
				case Comment::ABOVE: out.write( "\\an8" ); break;
				case Comment::BELOW: break; //Default
				case Comment::MIDDLE:
						QString move( "\\move(%1,%2,%3,%2)" );
						write( out, move.arg( 840 ).arg( v_pos ).arg( -200 ) );
					break;
			}
			
			//color TODO: alpha? \alpha&H80&
			if( color != white.color )
				write( out, QString("\\c&%1&").arg( color, 6, 16, QChar('0') ) );
			
			if( size != 28 )
				write( out, QString("\\fs%1").arg(size) );
			
			write( out, QString( "}%1\n" ).arg( content ) );
		}
};


bool chat_to_ass( xml_node root, QString out_path /*TODO: config */ ){
	if( !QFile::copy( "base.ass", out_path ) )
		qFatal( "Could not create ASS file!" );
	
	QFile ass( out_path );
	if( !ass.open( QIODevice::WriteOnly | QIODevice::Append ) )
		qFatal( "Could not write ASS file!" );
	
	//Load data
	vector<Comment> comments;
	for( auto chat : root.children( "chat" ) ){
		int vpos = chat.attribute( "vpos" ).as_int();
		comments.emplace_back( vpos, chat.attribute( "mail" ).value(), chat.child_value() );
	}
	
	//Randomness
	uniform_int_distribution<int> distribution( 20, 340 );
	uniform_int_distribution<int> display_dist( -100, 100 );
	default_random_engine generator( chrono::system_clock::now().time_since_epoch().count() );
	
	sort( comments.begin(), comments.end() );
	for( auto comment : comments ){
		//TODO: make sure they overlap as little as possible
		int v_pos = distribution( generator );
		
		double rand_display = display_dist( generator ) / 100.0;
		AssLine line( comment.time / 100.0, 3.0+rand_display, v_pos, comment.contents.c_str() );
		line.color = comment.color.color;
		switch( comment.size ){
			case Comment::SMALL:  line.size = 16; break; //TODO:
			case Comment::NORMAL: line.size = 28; break;
			case Comment::BIG:    line.size = 40; break;
		}
		line.pos = comment.pos;
		
		//TODO: fixed position
		
		line.write( ass );
	}
	
	return true;
}

