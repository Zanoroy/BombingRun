Bombing Run Game Specification
This is a C++ - designed to learn how to use AI to help create a graphical based game using sprits etc and will need to run on Linux and Windows.

The code is to be created using C++ self contained objects and allow for future expansion of weapons, bombs and targets.


Overview
Bombing Run is a 2D aerial combat strategy game where players control bombing aircraft to destroy military targets. The game features multiple maps, various bomb types, defensive fighter jets, ground air defence and dynamic destruction systems. The ground base should also be distructable which causes ground defence vechicals to be unable to cross location.

All vechicals should be sprites that can be changed to give different look and feel. Each map should be individual "level" the user can select to play on. 

Core Game Mechanics
Game States
Menu: Map selection screen with available missions
Playing: Active gameplay with aircraft, bombing, and combat
Maps & Scaling
City Map: Default urban environment (1.0x scale)
BattleGround Map: Large military airfield (0.25x scale - 4x zoom out)
Canvas & Display
Coordinate System: Standard 2D canvas with origin at top-left
Scaling: Dynamic scaling based on selected map
Aircraft Systems
Bomber Aircraft
Properties:

Position (x, y coordinates)
Type (determines bomb payload)
Target position for dropping bombs
Movement speed: varies by bomb type
Maximum bombs carried: varies by bomb configuration
Movement Patterns:

Spawn from random positions above screen (y < 0)
Fly toward target location on ground
Drop bombs when reaching target area
Continue flying and exit screen
Health System:

Bombers: 3 hits to destroy (multi-hit system)
Visual damage indicators: color changes from normal to orange/red
Smoke effects for damaged aircraft
Fighter Jets (Defensive Units)
Availability: BattleGround map only

Properties:

Health: 4 hits to destroy
Position (x, y coordinates)
Target position (patrol area)
Angle and speed
Ammunition: 579 bullets
Hit rate: 80% accuracy
Multi-hit damage system with visual indicators
Behavior:

Spawn from military base when bombers approach
Patrol around military airfield
Automatically engage bomber aircraft
Fire bullets at intercepted bombers
Return to base when no targets present
Combat System:

Bullet-based combat with projectile physics
Predictive targeting algorithm
Limited ammunition per fighter jet
Bullet collision detection with bomber aircraft
Bomb System
Bomb Configurations
Type	Max Bombs	Crater Size	Target Radius	Damage	Speed
100lb	10	15px	60px	1	6.0
250lb	8	20px	75px	2	5.5
500lb	6	25px	90px	3	5.0
1000lb	3	40px	120px	5	4.0
2000lb	1	80px	90px	10	0.8
4000lb	1	160px	60px	20	0.3
8000lb	1	320px	60px	40	0.2
Airstrike	15	15px	90px	1	5.0 (5 planes)
Bomb Physics
Drop Mechanics: Bombs drop from aircraft at target coordinates
Explosion System: Creates visual explosions with size based on bomb type
Crater Formation: Permanent craters remain after explosions
Collision Detection: Circle-rectangle collision for runway damage
Special Bomb Types
8000lb Bomb:

Largest crater with special three-ring visual effect
Highest damage output
Slowest aircraft speed
Creates massive ground impact
Airstrike:

Deploys 5 separate bomber aircraft
Each carries 15 smaller bombs
Coordinated attack pattern
Target Systems
Buildings (City Map)
Randomly generated civilian buildings
Various sizes and colors
Destructible by bomb impacts
Visual damage states
Military Base (BattleGround Map)
Runway System:

Health: 1000 HP (primary target)
Regeneration: Gradual health recovery over time
Damage Calculation: Based on bomb type and impact area
Victory Condition: Destroy runway to win mission
Military Buildings:

Hangars: Large rectangular structures
Fuel Tanks: Circular storage facilities
Radar Stations: Specialized circular buildings
Support Buildings: Various military structures
Destruction: Buildings can be permanently destroyed
Health Reduction: Each destroyed building reduces runway max health by 10 HP
Building Types:

Rectangular buildings (hangars, barracks, storage)
Circular buildings (fuel tanks, radar stations)
Special runway structure (primary target)
Color-coded by function
User Interface
Menu System
Map selection buttons
Bomb type selector
Play/Start mission button
Visual map previews
HUD Elements
Current Bomb Type: Display selected ordnance
Bombs Loaded: Available ammunition count
Planes Active: Number of bombers in flight
Runway Health Bar (BattleGround only): Visual health display
Runway Health Display
Health bar with color coding:
Green: >66% health
Orange: 33-66% health
Red: <33% health
Text display: "Current/Max (Buildings: Destroyed/Total)"
Only visible on BattleGround map
Visual & Audio Systems
Graphics Rendering
Background: Grass texture with random pattern
Aircraft: Detailed bomber and fighter jet sprites
Explosions: Multi-stage explosion animations
Craters: Persistent crater marks with aging effects
Buildings: Color-coded structures with damage states
Visual Effects
Explosion animations with multiple colors
Muzzle flashes from fighter jet weapons
Targeting circles showing bomb impact areas
Damage progression on aircraft (color changes)
Smoke effects for damaged units
Coordinate Scaling
Dynamic scaling based on selected map
BattleGround uses 0.25x scale for large area visibility
All visual elements scale proportionally
Input Controls
Mouse Controls
Click: Drop bombs at cursor position
Movement: Track cursor for targeting display
UI Interaction: Menu navigation and bomb selection
Keyboard Controls
Number Keys (1-7): Select bomb types (100lb through 8000lb)
A Key: Deploy airstrike
Spacebar: Deploy bomber with current bomb type
Game Physics
Movement Systems
Aircraft follow predetermined flight paths
Bullets use projectile physics with gravity
Collision detection for all interactive elements
Damage Systems
Multi-hit aircraft health system
Runway damage calculation with area effects
Building destruction with permanent effects
Regeneration Mechanics
Runway health slowly regenerates over time
Building damage is permanent
Aircraft respawn is trigger-based
Audio (Implementation Ready)
Explosion sound effects
Aircraft engine sounds
Weapon firing sounds
Menu interaction sounds
Background music/ambience
Win/Loss Conditions
Victory Conditions
BattleGround Map: Reduce runway health to 0
City Map: Destroy all target buildings (optional)
Failure Conditions
All bombers destroyed by fighter jets
Run out of ammunition without achieving objectives
Technical Architecture
Core Classes/Systems
Game Controller: Main game loop and state management
Aircraft Manager: Handle bomber and fighter jet systems
Weapon System: Bomb and bullet mechanics
Map Generator: Procedural building and base generation
Collision System: Detect impacts and interactions
UI Manager: Handle interface and HUD updates
Rendering System: Draw all visual elements with scaling
Data Structures
Aircraft Arrays: Store all active planes and fighters
Projectile Arrays: Track bombs and bullets in flight
Building Arrays: Manage destructible structures
Effect Arrays: Handle explosions and visual effects
Game Loop Structure
Update Phase:

Aircraft movement and AI
Projectile physics
Collision detection
Health/damage calculations
Effect animations
Render Phase:

Background and terrain
Buildings and structures
Aircraft and projectiles
Effects and UI elements
Performance Considerations
Efficient collision detection algorithms
Object pooling for frequently created/destroyed items
Optimized rendering with culling for off-screen elements
Memory management for arrays of game objects
Platform Requirements
Graphics: 2D canvas or equivalent rendering system
Input: Mouse and keyboard support
Performance: 60 FPS target with smooth animations
Resolution: Scalable interface design
Audio: Sound effect and music playback capability