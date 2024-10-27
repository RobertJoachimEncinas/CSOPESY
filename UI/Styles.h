/*
    This file defines all code related to the styling of the UI
*/

typedef enum TextColor {
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37,
    RESET = 0,
} TextColor;

TextColor stringToColor(const std::string& color) {
    if (color == "RED") return RED;
    if (color == "GREEN") return GREEN;
    if (color == "YELLOW") return YELLOW;
    if (color == "BLUE") return BLUE;
    if (color == "MAGENTA") return MAGENTA;
    if (color == "CYAN") return CYAN;
    if (color == "WHITE") return WHITE;
    return RESET; // Default to RESET if color not recognized
}