extern int currentScreen;
extern int currentBrightness;

void refreshScreen();
void changeScreen(int newScreen = -1);
void setBrightness(int newBrightness = 0);
void startupLog(char in_logMessage[65], int in_textSize);