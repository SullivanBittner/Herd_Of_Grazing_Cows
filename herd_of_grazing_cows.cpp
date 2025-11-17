#include "herd_of_grazing_cows.h"
#include <QPainter>        // For custom drawing
#include <QDateTime>       // For time calculations
#include <QDebug>          // For debug output
#include <cmath>           // For math functions
#include <functional>      // For std::function


// Field sizes vector declaration
// Larger numbers are more zoomed out
const QVector<int> Herd_of_Grazing_Cows::fieldSizes = {50, 25, 20, 10, 5, 4, 2, 1};
const QVector<int> GameDisplayWidget::fieldSizes = {50, 25, 20, 10, 5, 4, 2, 1};

// Upgrade implementation
// Constructor to initialize all upgrade variables
Herd_of_Grazing_Cows::Upgrade::Upgrade(const QString& name, double price, double multiplier,
                                std::function<void()> onBuy, const QString& displayText,
                                const QString& displayName, std::function<bool()> canBuy)
    : name(name), displayName(displayName), displayText(displayText),
    price(price), multiplier(multiplier), onBuy(onBuy), canBuy(canBuy), level(0)
{}


// Applies upgrade purchase effects and increases price
void Herd_of_Grazing_Cows::Upgrade::buy()
{
    // Check if upgrade can be purchased
    if (canBuy())
    {
        onBuy();                // Execute the upgrade effect
        level++;                // Increase upgrade level
        price *= multiplier;    // Increase price for next purchase
    }
}

// Gets display text for UI
QString Herd_of_Grazing_Cows::Upgrade::getDisplayText() const
{
    return displayText;
}


// Main class constructor
Herd_of_Grazing_Cows::Herd_of_Grazing_Cows(QWidget *parent)
    : QMainWindow(parent),                              // Initialize base QMainWindow class
    // Initialize all game state variables
    money(0), totalMoney(0),                            // Start with no money
    herdX(0), herdY(0),                                 // Herd starts at top left
    herdWidth(1), herdHeight(1),                        // Herd starts as 1 cow by 1 cow
    herdSpeed(1),                                       // 1 acre per day
    herdDirectionUp(false),                             // Start moving down
    growthAmount(4),                                    // 4 growth actions per day
    fieldSize(0),                                       // Start with largest field size
    dayRate(1000),                                      // 1 second per game day
    totalCleared(0),                                    // No grass cleared yet
    lastDay(0), superExtra(0), superDays(0),            // Time tracking
    gameDisplayWidget(nullptr), centralWidget(nullptr)  // UI
{
    setFixedSize(800, 600); // Window size 800x600 pixels

    // Create game systems
    initializeGame();      // Set up initial game state
    initializeUpgrades();  // Create all available upgrades
    createUI();            // Build the UI

    // Set up timer
    gameTimer = new QTimer(this);
    // Connect timer to gameUpdate
    connect(gameTimer, &QTimer::timeout, this, &Herd_of_Grazing_Cows::gameUpdate);
    // Sets timer to current dayRate
    gameTimer->start(dayRate);
}

// Destructor to delete allocated memory
Herd_of_Grazing_Cows::~Herd_of_Grazing_Cows()
{
    // Delete upgrades to prevent memory leak
    qDeleteAll(upgrades);
}

// Creates UI
void Herd_of_Grazing_Cows::createUI()
{

    // create central widget to contain all UI elements
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Main layout with left and right sections
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    // Left side game display
    gameDisplayWidget = new GameDisplayWidget(centralWidget);
    gameDisplayWidget->setFixedSize(width , height);

    // Right side controls and info
    QWidget* controlPanel = new QWidget(centralWidget);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);

    // Stats group
    QGroupBox* statsGroup = new QGroupBox("Game Stats", this);
    QVBoxLayout* statsLayout = new QVBoxLayout(statsGroup);

    // Label initializations
    moneyLabel = new QLabel("Money: $0.00", this);
    totalClearedLabel = new QLabel("Total Acres Cleared: 0", this);
    speedLabel = new QLabel("Herd Speed: 1", this);
    sizeLabel = new QLabel("Herd Size: 1x1", this);
    growthLabel = new QLabel("Growth Rate: 4", this);
    dayRateLabel = new QLabel("Day Rate: 1000ms", this);
    superDaysLabel = new QLabel("", this);

    // Add labels to stats layout
    statsLayout->addWidget(moneyLabel);
    statsLayout->addWidget(totalClearedLabel);
    statsLayout->addWidget(speedLabel);
    statsLayout->addWidget(sizeLabel);
    statsLayout->addWidget(growthLabel);
    statsLayout->addWidget(dayRateLabel);
    statsLayout->addWidget(superDaysLabel);

    // Upgrades group
    QGroupBox* upgradesGroup = new QGroupBox("Upgrades", this);
    QVBoxLayout* upgradesLayout = new QVBoxLayout(upgradesGroup);

    // Speed upgrade
    speedUpgradeButton = new QPushButton("Herd Speed Upgrade: $50", this);
    connect(speedUpgradeButton, &QPushButton::clicked, this, &Herd_of_Grazing_Cows::buySpeedUpgrade);

    // Size upgrade
    sizeUpgradeButton = new QPushButton("Herd Size Upgrade: $75", this);
    connect(sizeUpgradeButton, &QPushButton::clicked, this, &Herd_of_Grazing_Cows::buySizeUpgrade);

    // Field upgrade
    fieldUpgradeButton = new QPushButton("Field Size Upgrade: $150", this);
    connect(fieldUpgradeButton, &QPushButton::clicked, this, &Herd_of_Grazing_Cows::buyFieldUpgrade);

    // Growth upgrade
    growthUpgradeButton = new QPushButton("Growth Speed Upgrade: $10", this);
    connect(growthUpgradeButton, &QPushButton::clicked, this, &Herd_of_Grazing_Cows::buyGrowthUpgrade);

    // Day rate upgrade
    dayUpgradeButton = new QPushButton("Day Rate Upgrade: $5", this);
    connect(dayUpgradeButton, &QPushButton::clicked, this, &Herd_of_Grazing_Cows::buyDayUpgrade);

    // Adds buttons to upgrades layout
    upgradesLayout->addWidget(speedUpgradeButton);
    upgradesLayout->addWidget(sizeUpgradeButton);
    upgradesLayout->addWidget(fieldUpgradeButton);
    upgradesLayout->addWidget(growthUpgradeButton);
    upgradesLayout->addWidget(dayUpgradeButton);

    // Add groups to control layout
    controlLayout->addWidget(statsGroup);
    controlLayout->addWidget(upgradesGroup);
    controlLayout->addStretch();

    // Add left and right sides to main layout
    mainLayout->addWidget(gameDisplayWidget);
    mainLayout->addWidget(controlPanel);
}

// Initialize game function
void Herd_of_Grazing_Cows::initializeGame()
{
    generateField();                               // Creates initial field
    lastDay = QDateTime::currentMSecsSinceEpoch(); // QDateTime function to return current miliseconds
}

// Initialize upgrades function
void Herd_of_Grazing_Cows::initializeUpgrades()
{
    // Herd speed upgrade
    // Increases how many moves the herd makes per day
    upgrades.append(new Upgrade(
        "herdSpeed",           // Internal name
        speedBasePrice,        // Starting price: $50
        speedMultiplier,       // Price multiplier: 2.0 (doubles each purchase)
        [this](){ this->herdSpeed++; },  // Effect: increase herd speed by 1
        "acres/day",           // Display text
        "Herd Speed",          // User friendly name
        [this](){ return this->herdSpeed < 50; }  // Can buy until speed reaches 50
        ));

    // herd size upgrade
    // Increases the area the herd covers when moving
    upgrades.append(new Upgrade(
        "herdSize",
        sizeBasePrice,         // Starting price: $75
        sizeMultiplier,        // Price multiplier: 1.3 (30% increase)
        [this]()
        {
            // Alternate between increasing width and height
            if (this->herdWidth == this->herdHeight) {
                this->herdWidth++;   // Increase width if square
            }
            else
            {
                this->herdHeight++;  // Increase height if rectangular
            }
            // Reset position to top-left when size changes
            this->herdX = 0;
            this->herdY = 0;
        },
        "size",                // Display text
        "Herd Size",           // User friendly name
        [this]()
        {
            // Can't exceed field boundaries
            int maxSize = height / fieldSizes[this->fieldSize];
            return this->herdHeight < maxSize && this->herdWidth < maxSize;
        }
        ));

    // field size upgrade
    // Changes the zoom level
    upgrades.append(new Upgrade(
        "fieldSize",
        fieldBasePrice,        // Starting price: $150
        fieldMultiplier,       // Price multiplier: 2.5 (150% increase)
        [this]() {
            // Increase field size index (makes cells smaller)
            this->fieldSize = qMin(this->fieldSize + 1, fieldSizes.size() - 1);
            this->regenerateField();  // Recreate field with new cell size
        },
        "field size",          // Display text
        "Field Size",          // User friendly name
        [this]() {
            // Can buy until reaching smallest field size
            return this->fieldSize < fieldSizes.size() - 1;
        }
        ));

    // growth rate upgrade
    // Increases how much grass grows per day
    upgrades.append(new Upgrade(
        "growthRate",
        growthBasePrice,       // Starting price: $10
        growthMultiplier,      // Price multiplier: 1.15 (15% increase)
        [this]()
        {
            this->growthAmount += 2;  // Increase growth by 2 units per day
        },
        "growth/day",          // Display text
        "Growth Rate",         // User friendly name
        [this]()
        {
            // Can buy until growth reaches 100 per day
            return this->growthAmount < 100;
        }
        ));



    // day rate upgrade
    // Speeds up the game by reducing time between days
    upgrades.append(new Upgrade(
        "dayRate",
        dayBasePrice,          // Starting price: $5
        dayMultiplier,         // Price multiplier: 1.15 (15% increase)
        [this]()
        {
            // Reduce day rate by 15%, minimum 1ms
            this->dayRate = qMax(1, static_cast<int>(this->dayRate * 0.85));
        },
        "ms",                  // Display text
        "Day Rate",            // User friendly name
        [this]()
        {
            // Can buy until day rate reaches 1ms
            return this->dayRate > 1;
        }
        ));
}

// generate field function to create the field
void Herd_of_Grazing_Cows::generateField()
{
    // Calculate grid dimensions based on current field size
    int gridWidth = width / fieldSizes[fieldSize];
    int gridHeight = height / fieldSizes[fieldSize];

    // Resize the grid to match dimensions
    grid.resize(gridWidth);
    for (int i = 0; i < gridWidth; ++i)
    {
        grid[i].resize(gridHeight);  // Set up each column
        for (int j = 0; j < gridHeight; ++j)
        {
            // Initialize each cell with random growth level
            grid[i][j] = rand() % maxGrowth;
        }
    }
}


// Regenerate field function when field size changes
void Herd_of_Grazing_Cows::regenerateField()
{
    grid.clear();       // Clears field
    generateField();    // Create new field
}

// All herd activity for one day
void Herd_of_Grazing_Cows::herdDay()
{
    // Gets time since last day
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 timeDifference = currentTime - lastDay;
    lastDay = currentTime;

    // Uses extra time for super days
    superExtra += timeDifference - dayRate;
    if (superExtra > dayRate * 5)
    {
        superDays += static_cast<int>(superExtra / 5 / dayRate);
        superExtra = fmod(superExtra, dayRate * 5);
    }

    // Gets current grid dimensions
    int gridWidth = width / fieldSizes[fieldSize];
    int gridHeight = height / fieldSizes[fieldSize];

    // Herd movements and grass clearing
    // Number of moves based on herd speed
    for (int i = 0; i < herdSpeed; ++i)
    {

        for (int x = 0; x < herdWidth; ++x)
        {
            for (int y = 0; y < herdHeight; ++y)
            {
                // Get real herd position
                int tX = x + herdX;
                int tY = y + herdY;

                // Check if position is valid and grass is grown
                if (tX < gridWidth && tY < gridHeight && grid[tX][tY] >= 5)
                {
                    // Clears grass
                    grid[tX][tY] = 0;
                    // Gets money
                    double value = 1.0 * (superDays > 0 ? 5 : 1);
                    money += value;
                    totalMoney += value;
                    totalCleared++;

                    // Use up a super day if it is active
                    if (superDays > 0)
                    {
                        superDays--;
                    }
                }
            }
        }

        // Herd movement pattern
        if (herdDirectionUp)  // Moving upward
        {
            if (herdY > 0)  // Can move up
            {
                herdY--;
            }
            else if (herdX >= gridWidth - herdWidth)  // At top and right edge
            {
                // Reset to start position
                herdDirectionUp = false;
                herdX = 0;
                herdY = 0;
            }
            else  // At top but not at right edge
            {
                // Move right and start moving down
                herdX = qMin(herdX + herdWidth, gridWidth - herdWidth);
                herdDirectionUp = false;
            }
        }
        else  // Moving downward
        {
            if (herdY < gridHeight - herdHeight)  // Can move down
            {
                herdY++;
            }
            else if (herdX >= gridWidth - herdWidth)  // At bottom and right edge
            {
                // Reset to start position
                herdDirectionUp = false;
                herdX = 0;
                herdY = 0;
            }
            else  // At bottom but not at right edge
            {
                // Move right and start moving up
                herdX = qMin(herdX + herdWidth, gridWidth - herdWidth);
                herdDirectionUp = true;
            }
        }
    }

    // Update UI for all herd/day changes
    updateUI();
}

// Processes grass growth for one day
void Herd_of_Grazing_Cows::growthDay()
{
    int gridWidth = width / fieldSizes[fieldSize];
    int gridHeight = height / fieldSizes[fieldSize];

    // Grow grass multiple times based on growth rate
    for (int i = 0; i < growthAmount; ++i)
    {
        // Pick a random cell to grow
        int x = rand() % gridWidth;
        int y = rand() % gridHeight;

        // If grass isn't fully grown, increase its growth level
        if (grid[x][y] < maxGrowth)
        {
            grid[x][y] = qMin(maxGrowth, grid[x][y] + 1);
        }
    }
}


// Called by timer to advance game state
void Herd_of_Grazing_Cows::gameUpdate()
{
    herdDay();    // Handle herd movement and grass clearing
    growthDay();  // Handle grass growth

    // Update visual display with current game state
    if (gameDisplayWidget)
    {
        gameDisplayWidget->setGameData(&grid, fieldSize, herdX, herdY, herdWidth, herdHeight);
    }

    // Refresh UI elements
    updateUI();
}

// Function to update all UI
void Herd_of_Grazing_Cows::updateUI()
{
    // update labels
    moneyLabel->setText(QString("Money: $%1").arg(money, 0, 'f', 2));
    totalClearedLabel->setText(QString("Total Cleared: %1").arg(totalCleared, 0, 'f', 0));
    speedLabel->setText(QString("Herd Speed: %1").arg(herdSpeed));
    sizeLabel->setText(QString("Herd Size: %1x%2").arg(herdWidth).arg(herdHeight));
    growthLabel->setText(QString("Growth Rate: %1").arg(growthAmount));
    dayRateLabel->setText(QString("Day Rate: %1ms").arg(dayRate));

    // Show super days count if any are active
    if (superDays > 0)
    {
        superDaysLabel->setText(QString("SUPER DAYS: %1").arg(superDays));
        superDaysLabel->setStyleSheet("color: red; font-weight: bold;");  // Highlight
    }
    else
    {
        superDaysLabel->setText("");  // Hide when no super days
    }

    // Update upgrade buttons
    // For each upgrade, update button text and enable/disable state

    // Herd Speed Upgrade Button
    Upgrade* speedUpgrade = getUpgrade("herdSpeed");
    if (speedUpgrade)
    {
        // Set button text with current price and level
        QString buttonText = speedUpgrade->canBuy() ?
                                 QString("Herd Speed Upgrade: $%1 (Lvl %2)")
                                     .arg(speedUpgrade->price, 0, 'f', 0)
                                     .arg(speedUpgrade->level) :
                                 "Herd Speed Upgrade: MAXED";
        speedUpgradeButton->setText(buttonText);

        // Enable button only if upgrade is available and affordable
        speedUpgradeButton->setEnabled(speedUpgrade->canBuy() && money >= speedUpgrade->price);
    }
    // same logic is applied for all upgrades

    // Herd Size Upgrade Button
    Upgrade* sizeUpgrade = getUpgrade("herdSize");
    if (sizeUpgrade)
    {
        QString buttonText = sizeUpgrade->canBuy() ?
                                 QString("Herd Size Upgrade: $%1 (Lvl %2)")
                                     .arg(sizeUpgrade->price, 0, 'f', 0)
                                     .arg(sizeUpgrade->level) :
                                 "Herd Size Upgrade: MAXED";
        sizeUpgradeButton->setText(buttonText);
        sizeUpgradeButton->setEnabled(sizeUpgrade->canBuy() && money >= sizeUpgrade->price);
    }

    // Field Size Upgrade Button
    Upgrade* fieldUpgrade = getUpgrade("fieldSize");
    if (fieldUpgrade)
    {
        QString buttonText = fieldUpgrade->canBuy() ?
                                 QString("Field Size Upgrade: $%1 (Lvl %2)")
                                     .arg(fieldUpgrade->price, 0, 'f', 0)
                                     .arg(fieldUpgrade->level) :
                                 "Field Size Upgrade: MAXED";
        fieldUpgradeButton->setText(buttonText);
        fieldUpgradeButton->setEnabled(fieldUpgrade->canBuy() && money >= fieldUpgrade->price);
    }

    // Growth Rate Upgrade Button
    Upgrade* growthUpgrade = getUpgrade("growthRate");
    if (growthUpgrade)
    {
        QString buttonText = growthUpgrade->canBuy() ?
                                 QString("Growth Rate Upgrade: $%1 (Lvl %2)")
                                     .arg(growthUpgrade->price, 0, 'f', 0)
                                     .arg(growthUpgrade->level) :
                                 "Growth Rate Upgrade: MAXED";
        growthUpgradeButton->setText(buttonText);
        growthUpgradeButton->setEnabled(growthUpgrade->canBuy() && money >= growthUpgrade->price);
    }

    // Day Rate Upgrade Button
    Upgrade* dayUpgrade = getUpgrade("dayRate");
    if (dayUpgrade)
    {
        QString buttonText = dayUpgrade->canBuy() ?
                                 QString("Day Rate Upgrade: $%1 (Lvl %2)")
                                     .arg(dayUpgrade->price, 0, 'f', 0)
                                     .arg(dayUpgrade->level) :
                                 "Day Rate Upgrade: MAXED";
        dayUpgradeButton->setText(buttonText);
        dayUpgradeButton->setEnabled(dayUpgrade->canBuy() && money >= dayUpgrade->price);
    }
}

// Get upgrade by its internal name
Herd_of_Grazing_Cows::Upgrade* Herd_of_Grazing_Cows::getUpgrade(const QString& name)
{
    // Searches all upgrades
    for (Upgrade* upgrade : upgrades)
    {
        if (upgrade->name == name)
        {
            return upgrade;
        }
    }
    return nullptr; // Returns nothing if upgrade is not found
}

// The following buy upgrade functions use previous get upgrade function
// All are called when the upgrade buttons are clicked

// Purchase herd speed upgrade
void Herd_of_Grazing_Cows::buySpeedUpgrade()
{
    if (Upgrade* upgrade = getUpgrade("herdSpeed"))
    {
        // Check if player can afford and upgrade is available
        if (money >= upgrade->price && upgrade->canBuy())
        {
            upgrade->buy();          // Apply upgrade effects
            money -= upgrade->price; // Deduct cost
            if(money < 0)            // Safety check (shouldn't happen)
                money = 0;
            updateUI();              // Refresh display
        }
    }
}
// logic is the same for all other buyUpgrade functions

// Purchase herd size upgrade
void Herd_of_Grazing_Cows::buySizeUpgrade()
{
    if (Upgrade* upgrade = getUpgrade("herdSize"))
    {
        if (money >= upgrade->price && upgrade->canBuy())
        {
            upgrade->buy();
            money -= upgrade->price;
            if(money < 0)
                money = 0;
            updateUI();
        }
    }
}

// Purchase field size upgrade
void Herd_of_Grazing_Cows::buyFieldUpgrade()
{
    if (Upgrade* upgrade = getUpgrade("fieldSize"))
    {
        if (money >= upgrade->price && upgrade->canBuy())
        {
            upgrade->buy();
            money -= upgrade->price;
            if(money < 0)
                money = 0;
            updateUI();
        }
    }
}

// Purchase growth rate upgrade
void Herd_of_Grazing_Cows::buyGrowthUpgrade()
{
    if (Upgrade* upgrade = getUpgrade("growthRate"))
    {
        if (money >= upgrade->price && upgrade->canBuy())
        {
            upgrade->buy();
            money -= upgrade->price;
            if(money < 0)
                money = 0;
            updateUI();
        }
    }
}

// Purchase day rate upgrade
void Herd_of_Grazing_Cows::buyDayUpgrade()
{
    if (Upgrade* upgrade = getUpgrade("dayRate"))
    {
        if (money >= upgrade->price && upgrade->canBuy())
        {
            upgrade->buy();
            money -= upgrade->price;
            if(money < 0)
                money = 0;
            updateUI();
        }
    }
}


// Called when widget needs to be redrawn
void Herd_of_Grazing_Cows::paintEvent(QPaintEvent* event)
{
    // Empty because drawing is handled by the GameDisplayWidget class
    QPainter painter(this);
}
