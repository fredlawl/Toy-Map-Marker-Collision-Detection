/*
* Key Usage:
*	- Page Up: Increment the seed
*	- Page Down: Decrement the seed
*	- h: Toggles between showing/hiding all labels (excluding those that collide)
*	- ESCAPE: Closes app
*	- s: Allows user to change the seed
*/

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <string>
#include <vector>
#include <algorithm>

#define WEST olc::vi2d(1, 0)
#define EAST olc::vi2d(0, 0)

#define APP_STATE_NORMAL 1
#define APP_STATE_SEED_JUMP 2

const std::string __USStates[] = {
	"Alabama", "Alaska", "Arizona", "Arkansas", "California",
	"Colorado", "Connecticut", "Delaware", "Florida", "Georgia",
	"Hawaii", "Idaho", "Illinois", "Indiana", "Iowa",
	"Kansas", "Kentucky", "Louisiana", "Maine", "Maryland",
	"Massachusetts", "Michigan", "Minnesota", "Mississippi", "Missouri",
	"Montana", "Nebraska", "Nevada", "New Hampshire", "New Jersey",
	"New Mexico", "New York", "North Carolina", "North Dakota", "Ohio",
	"Oklahoma", "Oregon", "Pennsylvania", "Rhode Island", "South Carolina",
	"South Dakota", "Tennessee", "Texas", "Utah", "Vermont",
	"Virginia", "Washington", "West Virginia", "Wisconsin", "Wyoming",
};

/*
* Map marker
*/
struct Marker {
	olc::vi2d position;
	olc::vi2d dimensions;
	std::string title;
};

/*
* Represents a label for a marker
*/
struct MarkerLabel {
	Marker marker;
	olc::vi2d direction;
	bool hidden = false;
	bool drawn = false;
	olc::vi2d position() const {
		olc::vi2d position;
		position.x = (this->direction.x) ? 
			this->marker.position.x - this->dimensions().x : 
			this->marker.position.x + this->marker.dimensions.x;
		position.y = this->marker.position.y + this->marker.dimensions.y / 2 - this->dimensions().y / 2;
		return position;
	}
	olc::vi2d dimensions() const {
		return olc::vi2d(this->marker.title.size() * 8, 8);
	}
};

/*
* Creates a hitbox for the markerLabel depending on position + dimensions
*/
struct MarkerLabelHitbox {
	olc::vi2d position;
	olc::vi2d dimensions;
	MarkerLabelHitbox(const struct MarkerLabel& markerLabel) {
		this->position = olc::vi2d(
			(markerLabel.direction.x && !markerLabel.hidden) ? markerLabel.position().x : markerLabel.marker.position.x,
			markerLabel.marker.position.y
		);
		this->dimensions = olc::vi2d(
			(!markerLabel.hidden) ? markerLabel.dimensions().x + markerLabel.marker.dimensions.x : markerLabel.marker.dimensions.x,
			markerLabel.marker.dimensions.y
		);
	}
	bool collidesWith(const struct MarkerLabelHitbox& b) {
		return this->position.x + this->dimensions.x > b.position.x &&
			this->position.y + this->dimensions.y > b.position.y &&
			this->position.x < b.position.x + b.dimensions.x &&
			this->position.y < b.position.y + b.dimensions.y;
	}
	bool operator==(const MarkerLabelHitbox& h) const {
		return this->position == h.position && this->dimensions == h.dimensions;
	}
};

class Example : public olc::PixelGameEngine
{
private:
	size_t seed = 1;
	std::vector<struct MarkerLabel> markerLabels;
	olc::vi2d directionPreference = WEST;
	bool hideAllLabels = false;
	int appState = APP_STATE_NORMAL;
	std::string seedTextBuffer;

public:

	Example()
	{
		sAppName = "Collision Example";
	}

public:
	bool OnUserCreate() override
	{
		this->createMarkers();
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Quickly close
		if (GetKey(olc::ESCAPE).bPressed) {
			return false;
		}

		Clear(olc::BLACK);

		switch (this->appState) {
		case APP_STATE_NORMAL:
			return this->appStateNormal(fElapsedTime);
		case APP_STATE_SEED_JUMP:
			return this->appStateSeedJump(fElapsedTime);
		}

		// Window Border
		DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, olc::DARK_BLUE);

		return true;
	}

	void createMarkers() {
		srand(this->seed);
		this->directionPreference = (this->seed & 0x01) ? WEST : EAST;
		const olc::vi2d markerDimensions(24, 44);

		markerLabels.clear();

		// Create markers
		for (int i = 0; i < 20; i++) {
			struct Marker marker;
			struct MarkerLabel label;

			marker.title = __USStates[i % 50];
			marker.dimensions = markerDimensions;
			marker.position = olc::vi2d(
				markerDimensions.x + (rand() % ((ScreenWidth() + 1) - markerDimensions.x)) - markerDimensions.x,
				markerDimensions.y + (rand() % ((ScreenHeight() + 1) - markerDimensions.y)) - markerDimensions.y
			);

			label.marker = marker;
			label.direction = this->directionPreference;
			label.hidden = this->hideAllLabels;
			markerLabels.push_back(label);
		}

		// Run collision detection
		for (struct MarkerLabel& a : markerLabels) {
			for (struct MarkerLabel& b : markerLabels) {
				struct MarkerLabelHitbox aHitbox(a);
				struct MarkerLabelHitbox bHitbox(b);

				if (aHitbox == bHitbox) {
					continue;
				}

				if (aHitbox.collidesWith(bHitbox) && b.drawn) {
					a.hidden = true;
				}
			}

			a.drawn = true;
		}
	}

	bool appStateNormal(float fElapsedTime) {

		if (GetKey(olc::Key::H).bPressed) {
			this->hideAllLabels = !this->hideAllLabels;
			this->createMarkers();
		}

		if (GetKey(olc::Key::PGUP).bPressed) {
			srand(++this->seed);
			this->createMarkers();
		}

		if (GetKey(olc::Key::PGDN).bPressed) {
			this->seed = std::max<size_t>(1, --this->seed);
			this->createMarkers();
		}

		if (GetKey(olc::Key::S).bPressed) {
			seedTextBuffer.clear();
			this->appState = APP_STATE_SEED_JUMP;
			return this->OnUserUpdate(fElapsedTime);
		}

		this->drawMap(fElapsedTime);

		// Seed counter
		DrawString(10, 10, "Seed: " + std::to_string(this->seed), olc::YELLOW, 3);

		return true;
	}

	bool appStateSeedJump(float fElapsedTime) {
		if (GetKey(olc::Key::ENTER).bPressed || GetKey(olc::Key::RETURN).bPressed) {
			size_t newSeed = this->seed;
			this->appState = APP_STATE_NORMAL;

			if (this->seedTextBuffer.size() > 0) {
				newSeed = std::stoi(this->seedTextBuffer);
			}

			this->seed = std::max<size_t>(1, newSeed);
			this->createMarkers();
			return this->OnUserUpdate(fElapsedTime);
		}

		if (GetKey(olc::Key::K0).bPressed || GetKey(olc::Key::NP0).bPressed) {
			this->seedTextBuffer.append(1, '0');
		}

		if (GetKey(olc::Key::K1).bPressed || GetKey(olc::Key::NP1).bPressed) {
			this->seedTextBuffer.append(1, '1');
		}

		if (GetKey(olc::Key::K2).bPressed || GetKey(olc::Key::NP2).bPressed) {
			this->seedTextBuffer.append(1, '2');
		}

		if (GetKey(olc::Key::K3).bPressed || GetKey(olc::Key::NP3).bPressed) {
			this->seedTextBuffer.append(1, '3');
		}

		if (GetKey(olc::Key::K4).bPressed || GetKey(olc::Key::NP4).bPressed) {
			this->seedTextBuffer.append(1, '4');
		}

		if (GetKey(olc::Key::K5).bPressed || GetKey(olc::Key::NP5).bPressed) {
			this->seedTextBuffer.append(1, '5');
		}

		if (GetKey(olc::Key::K6).bPressed || GetKey(olc::Key::NP6).bPressed) {
			this->seedTextBuffer.append(1, '6');
		}

		if (GetKey(olc::Key::K7).bPressed || GetKey(olc::Key::NP7).bPressed) {
			this->seedTextBuffer.append(1, '7');
		}

		if (GetKey(olc::Key::K8).bPressed || GetKey(olc::Key::NP8).bPressed) {
			this->seedTextBuffer.append(1, '8');
		}

		if (GetKey(olc::Key::K9).bPressed || GetKey(olc::Key::NP9).bPressed) {
			this->seedTextBuffer.append(1, '9');
		}

		// Make use of the delete key
		if (GetKey(olc::Key::BACK).bPressed) {
			if (this->seedTextBuffer.size() > 0) {
				this->seedTextBuffer.pop_back();
			}
		}
		
		this->drawMap(fElapsedTime);

		// Seed counter
		DrawString(10, 10, "Seed: " + seedTextBuffer + "|", olc::YELLOW, 3);

		return true;
	}

	void drawMap(float fElapsedTime) {

		for (struct MarkerLabel l : markerLabels) {
			struct MarkerLabelHitbox hitBox = MarkerLabelHitbox(l);

			// Marker
			FillRect(l.marker.position, l.marker.dimensions, olc::RED);

			// Label
			if (!l.hidden) {
				DrawRect(l.position(), l.dimensions(), olc::WHITE);
				DrawString(l.position(), l.marker.title, olc::YELLOW);
			}

			// Hitbox
			DrawRect(hitBox.position, hitBox.dimensions, olc::GREEN);
		}
	}
};

int main()
{
	Example demo;
	if (demo.Construct(800, 600, 1, 1))
		demo.Start();

	return 0;
}