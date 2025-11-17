#ifndef HERD_OF_GRAZING_COWS_H
#define HERD_OF_GRAZING_COWS_H

#include <QMainWindow>      // Main application window
#include <QTimer>           // Timer for game updates
#include <QVector>          // Dynamic array container
#include <QColor>           // Color representation
#include <QPushButton>      // Clickable button widget
#include <QLabel>           // Text display widget
#include <QVBoxLayout>      // Vertical layout manager
#include <QHBoxLayout>      // Horizontal layout manager
#include <QGroupBox>        // Group container with title
#include <QPainter>         // 2D painting functionality

// Qt namespace declaration for UI classes
QT_BEGIN_NAMESPACE
namespace Ui {
class Herd_of_Grazing_Cows;
}
QT_END_NAMESPACE

// Forward declaration
class GameDisplayWidget;

// Main application class inheriting from QMainWindow
class Herd_of_Grazing_Cows : public QMainWindow
{
    // Qt macro to include signals and slots
    Q_OBJECT

public:
    // constructor to create the main window
    explicit Herd_of_Grazing_Cows(QWidget *parent = nullptr);

    // deconstructor to delete new data
    ~Herd_of_Grazing_Cows();

    // methods to get values for the game state
    double getmoney() const { return money; }
    double getTotalMoney() const { return totalMoney; }

private:
    // default ui class pointer
    Ui::Herd_of_Grazing_Cows *ui;

    // constant values
    static const int width = 500;           // Game display width in pixels
    static const int height = 500;          // Game display height in pixels
    static constexpr int maxGrowth = 15;    // Maximum grass growth level
    static const QVector<int> fieldSizes;   // Available field sizes for zoom levels

    // money variables
    double money;        // Current available money for purchases
    double totalMoney;   // Total money earned over game lifetime

    // forward declaration
    struct Upgrade;

    // herd values
    QVector<QVector<int>> grid;  // 2D grid representing the field, each cell has grass growth level 0-15
    int herdX, herdY;            // Current position of the herd, (0,0) is at the top-left corner
    int herdWidth, herdHeight;   // Size of the herd in grid cells
    int herdSpeed;               // How many moves the herd makes per day
    bool herdDirectionUp;        // true = moving up, false = moving down

    // field values
    int growthAmount;    // How much grass grows per day
    int fieldSize;       // Current zoom level for fieldSizes vector
    int dayRate;         // Milliseconds between game days
    double totalCleared; // Total number of grass tiles cleared over game lifetime

    qint64 lastDay;      // Last time a game day was processed
    double superExtra;   // Accumulated extra time for super days calculation
    int superDays;       // Bonus days that give 5 times the money

    // upgrades
    QVector<Upgrade*> upgrades;  // List of available upgrades

    // upgrade base prices
    const double growthBasePrice = 10;   // Cost to increase growth rate
    const double speedBasePrice = 50;    // Cost to increase herd speed
    const double sizeBasePrice = 75;     // Cost to increase herd size
    const double fieldBasePrice = 150;   // Cost to change field size
    const double dayBasePrice = 5;       // Cost to speed up game days

    // upgrade price multipliers
    const double growthMultiplier = 1.15;  // 15% price increase
    const double speedMultiplier = 2.0;    // 100% price increase
    const double sizeMultiplier = 1.3;     // 30% price increase
    const double fieldMultiplier = 2.5;    // 150% price increase
    const double dayMultiplier = 1.15;     // 15% price increase

    // UI
    GameDisplayWidget* gameDisplayWidget;  // Custom widget for game visual
    QWidget* centralWidget;                // Main container for all UI elements

    // label widgets
    QLabel* moneyLabel = nullptr;         // Shows current money
    QLabel* totalClearedLabel = nullptr;  // Shows total cleared tiles over game lifetime
    QLabel* speedLabel = nullptr;         // Shows herd speed
    QLabel* sizeLabel = nullptr;          // Shows herd dimensions
    QLabel* growthLabel = nullptr;        // Shows growth rate
    QLabel* dayRateLabel = nullptr;       // Shows game speed
    QLabel* superDaysLabel = nullptr;     // Shows bonus days count

    // button widgets for upgrades
    QPushButton* speedUpgradeButton;   // Button to buy speed upgrade
    QPushButton* sizeUpgradeButton;    // Button to buy size upgrade
    QPushButton* fieldUpgradeButton;   // Button to buy field upgrade
    QPushButton* growthUpgradeButton;  // Button to buy growth upgrade
    QPushButton* dayUpgradeButton;     // Button to buy day rate upgrade

    // upgrade structure to define upgrades and how they behave
    struct Upgrade {
        // upgrade properties
        QString name;                   // Internal identifier not seen in UI
        QString displayName;            // Name in UI
        QString displayText;            // Description text for UI
        double price;                   // Current cost to purchase
        double multiplier;              // Price increase multiplier after purchase
        std::function<void()> onBuy;    // Function called when upgrade is purchased
        std::function<bool()> canBuy;   // Function that checks if upgrade is available
        int level;                      // Current upgrade level

        // Constructor that initializes all upgrade properties
        Upgrade(const QString& name, double price, double multiplier, std::function<void()> onBuy,
                const QString& displayText, const QString& displayName, std::function<bool()> canBuy);

        // Struct functions
        void buy();                         // Applies the upgrade effects and increases price
        QString getDisplayText() const;     // Returns the display text for UI
    };

    // Game timer that triggers regular updates
    QTimer* gameTimer;

    // Game initialization functions
    void initializeGame();      // Sets up initial game state
    void initializeUpgrades();  // Creates all available upgrades
    void createUI();            // Builds the user interface

    // Game logic functions
    void generateField();                       // Creates a new field
    void regenerateField();                     // Clears and recreates the field
    void herdDay();                             // Processes herd movement and grass clearing per day
    void growthDay();                           // Processes grass growth per day
    Upgrade* getUpgrade(const QString& name);   // Finds upgrade by name
    void updateUI();                             // Refreshes all UI elements

// private slot functions initialization
private slots:                   // All called automatically when signals are received
    void gameUpdate();           // Called by timer to advance game state
    void buySpeedUpgrade();      // Called when speed upgrade button is clicked
    void buySizeUpgrade();       // Called when size upgrade button is clicked
    void buyFieldUpgrade();      // Called when field upgrade button is clicked
    void buyGrowthUpgrade();     // Called when growth upgrade button is clicked
    void buyDayUpgrade();        // Called when day rate upgrade button is clicked

// protected function initialization
protected:
    // Called automatically when widget needs to be redrawn
    // overridden to customize painting
    void paintEvent(QPaintEvent* event) override;

// signal functions initialization
signals:                                // Emit signals for slots
    void moneyChanged(double money);    // Signal emitted when money amount changes
};



// GameDisplayWidget implementation
// Custom widget responsible for visualizing the game state
class GameDisplayWidget : public QWidget
{
    // Qt macro to include signals and slots
    Q_OBJECT

public:
    // Constructor that creates the display widget
    explicit GameDisplayWidget(QWidget *parent = nullptr) : QWidget(parent) {}

    // Called when game state changes to display updated game state
    void setGameData(const QVector<QVector<int>>* grid, int fieldSize,
                     int herdX, int herdY, int herdWidth, int herdHeight)
    {
        m_grid = grid;             // Pointer to the game grid
        m_fieldSize = fieldSize;   // Current field size/zoom level
        m_herdX = herdX;           // Herd X position
        m_herdY = herdY;           // Herd Y position
        m_herdWidth = herdWidth;   // Herd width in cells
        m_herdHeight = herdHeight; // Herd height in cells
        update();                  // Schedule a repaint
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        // Marks event parameter as unused
        Q_UNUSED(event);

        // Creates a painter object to draw on this widget
        QPainter painter(this);
        // For smooth edges:
        painter.setRenderHint(QPainter::Antialiasing);

        // If no grid data is available, draw empty green field
        if (!m_grid || m_grid->isEmpty())
        {
            painter.fillRect(rect(), Qt::darkGreen);
            return;
        }

        // Get grid dimensions based on current field size
        int gridWidth = m_grid->size();
        int gridHeight = (*m_grid)[0].size();
        int fieldSizePx = fieldSizes[m_fieldSize];

        // Draw grass fields
        for (int x = 0; x < gridWidth; ++x)
        {
            // Get grass color based on growth level
            // Higher growth = darker green
            for (int y = 0; y < gridHeight; ++y)
            {
                double ratio = static_cast<double>((*m_grid)[x][y]) / maxGrowth;
                int green = 100 + static_cast<int>(155 * ratio);
                QColor color(0, green, 0);

                // Draw the grass tile
                painter.fillRect(x * fieldSizePx, y * fieldSizePx,
                                 fieldSizePx, fieldSizePx, color);

                // Draw dark green grid lines around the tile
                painter.setPen(QColor(0, 80, 0));
                painter.drawRect(x * fieldSizePx, y * fieldSizePx,
                                 fieldSizePx, fieldSizePx);
            }
        }

        // Draw herd as a brown rectangle
        painter.fillRect(m_herdX * fieldSizePx, m_herdY * fieldSizePx,
                         m_herdWidth * fieldSizePx, m_herdHeight * fieldSizePx,
                         QColor(101, 67, 33));

        // Draw white border around the herd
        painter.setPen(Qt::white);
        painter.drawRect(m_herdX * fieldSizePx, m_herdY * fieldSizePx,
                         m_herdWidth * fieldSizePx, m_herdHeight * fieldSizePx);
    }

private:
    // Game grid from main class
    const QVector<QVector<int>>* m_grid = nullptr;

    // Display values for field and herd
    int m_fieldSize = 0;                    // Field size/zoom level
    int m_herdX = 0, m_herdY = 0;           // Herd position
    int m_herdWidth = 1, m_herdHeight = 1;  // Herd size


    // Constants from main class
    static const QVector<int> fieldSizes;  // Available field sizes
    static constexpr int maxGrowth = 15;   // Maximum grass growth level
};

#endif // HERD_OF_GRAZING_COWS_H
