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

#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QDebug>

#include "pugixml/pugixml.hpp"
using namespace pugi;

void create_chapters( const char* name, long double start, long double end, long double stop, unsigned amount=100 );
bool chat_to_ass( xml_node root, QString out_path /*TODO: config */ );

int main(int argc, char* argv[] ){
	//create_chapters( "Oh oh oh", 19.999800000, 57.832755010, 60.160, 1000 );
	//return 0;
	
	//NOTE: This handles encoding for us, and also makes sure we can get unicode input (non-UTF8 systems)
	QCoreApplication app( argc, argv );
	if( app.arguments().count() != 2 )
		qFatal( "Usage: nicochat2ass filepath" );
	
	//NOTE: while it could have been much simpler, fopen can't open unicode paths on non-UTF8 systems.
	//Thus we use Qt which uses system-specific APIs for us
	QFile xml( app.arguments()[1] );
	if( !xml.open( QIODevice::ReadOnly ) )
		qFatal( "Couldn't open file!" );
	
	//Read xml file
	QByteArray contents = xml.readAll(); //NOTE: must be in memory for pugixml
	xml_document doc;
	auto error = doc.load_buffer_inplace( contents.data(), contents.size() ); //TODO: const
	if( !error )
		qFatal( error.description() );
	
	chat_to_ass( doc.child( "packet" ), "out.ass" );
	
	return 0;
}