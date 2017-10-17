#include <cmath>
#include <ctime>
#include <cstdlib>
#include <string>
#include <iostream>
//
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

sf::Vector2u imageSize;

template< typename T >
float getDistance( sf::Vector2< T > const &point1, sf::Vector2< T > const &point2 )
{
	return std::sqrt( std::pow( point1.x - point2.x, 2 ) + std::pow( point1.y - point2.y, 2 ));
}

bool isPointValid( sf::Vector2i const &point )
{
	return point.x >= 0 && point.y >= 0 &&
		point.x < static_cast< int >( imageSize.x ) && point.y < static_cast< int >( imageSize.y );
}

sf::Vector2i getRandomPixel()
{
	int x = std::rand() % imageSize.x;
	int y = std::rand() % imageSize.y;
	return sf::Vector2i( x, y );
}

sf::Color getRandomColor( sf::Image const &image )
{
	auto randomPixel = getRandomPixel();
	return image.getPixel( randomPixel.x, randomPixel.y );
}

sf::Color getRandomColor()
{
	return sf::Color( std::rand() % 256, std::rand() % 256, std::rand() % 256, std::rand() % 256 );
}

sf::Image &updateBrush( sf::Image &brush, sf::Color const &brushColor, float const &brushRadius )
{
	int brushCenter = brushRadius + 0.5f;
	for( int iY = brushCenter - brushRadius; iY <= brushCenter + brushRadius; iY++ )
	{
		for( int iX = brushCenter - brushRadius; iX <= brushCenter + brushRadius; iX++ )
		{
			if( getDistance(sf::Vector2i( iX, iY ), sf::Vector2i( brushCenter, brushCenter )) <= brushRadius )
			{
				brush.setPixel( iX, iY, brushColor );
			}
		}
	}
	return brush;
}

float fitness( sf::Image const &image1, sf::IntRect const &scope1, sf::Image const &image2, sf::IntRect const &scope2 )
{
	float fitness = 0;
	float change = 1.f / (scope1.width * scope1.height);
	for( int iY1 = scope1.top, iY2 = scope2.top; iY1 < scope1.top + scope1.height; iY1++, iY2++ )
	{
		for( int iX1 = scope1.left, iX2 = scope2.left; iX1 < scope1.left + scope1.width; iX1++, iX2++ )
		{
			auto color1 = image1.getPixel( iX1, iY1 );
			auto color2 = image2.getPixel( iX2, iY2 );
			float r = 1.f - (std::abs( color1.r - color2.r ) / 255.f);
			float g = 1.f - (std::abs( color1.g - color2.g ) / 255.f);
			float b = 1.f - (std::abs( color1.b - color2.b ) / 255.f);
			float a = 1.f - (std::abs( color1.a - color2.a ) / 255.f);
			fitness += change * ((r + g + b + a) / 4.f);
		}
	}
	return fitness;
}

sf::Vector2i getBrushPosition( sf::Vector2i const &brushSize )
{
	sf::Vector2i result;
	do
	{
		result = getRandomPixel();
	}
	while( result.x < 0 || result.y < 0 ||
		result.x + brushSize.x >= static_cast< int >( imageSize.x ) ||
		result.y + brushSize.y >= static_cast< int >( imageSize.y ));
	return result;
}

int main( int argc, char *argv[] )
{
	std::srand( std::time( nullptr ));
	std::string programName = argv[ 0 ];
	if( argc < 4 )
	{
		std::cout << "Usage: " + programName + " INPUT OUTPUT ITERATIONS BRUSH_SIZE" << std::endl;
		return 1;
	}
	std::string sourcePath = argv[ 1 ];
	std::string outputPath = argv[ 2 ];
	unsigned iterations = std::stoi( argv[ 3 ]);
	float brushRadius = std::stoi( argv[ 4 ]);

	sf::RenderWindow window( sf::VideoMode( 800, 600 ), "shapeImage" );

	sf::Image sourceImage;
	sourceImage.loadFromFile( sourcePath );
	imageSize = sourceImage.getSize();

	sf::Image image1;
	image1.create( imageSize.x, imageSize.y, sf::Color::Transparent );

	sf::RectangleShape shape({ static_cast< float >( window.getSize().x ),
		static_cast< float >( window.getSize().y )});
	sf::Texture texture;

	sf::Vector2i brushSize;
	sf::Image brush;

	sf::Event event;

	bool noRender = false;

	brushSize.x = (brushRadius * 2 ) + 1.5f;
	brushSize.y = brushSize.x;
	for( unsigned i = 0; i < iterations; i++ )
	{
		brush.create( brushSize.x, brushSize.y, sf::Color::Transparent );
		auto brushColor = getRandomColor( sourceImage );
		auto brushPosition = getBrushPosition( brushSize );
		sf::IntRect brushSquare( brushPosition, brushSize );
		brush.copy( image1, 0, 0, brushSquare, true );
		updateBrush( brush, brushColor, brushRadius );
		if( fitness( brush, sf::IntRect({ 0, 0 }, brushSize ), sourceImage, brushSquare ) >
			fitness( image1, brushSquare, sourceImage, brushSquare ))
		{
			image1.copy( brush, brushPosition.x, brushPosition.y, sf::IntRect( 0, 0, 0, 0 ), true );
		}
		if( !noRender )
		{
			texture.loadFromImage( image1 );
			shape.setTexture( &texture );
			window.draw( shape );

			window.display();
		}

		if( window.pollEvent( event ))
		{
			if( event.type == sf::Event::Closed )
			{
				break;
			}
		}
	}
	image1.saveToFile( outputPath );
	return 0;
}
