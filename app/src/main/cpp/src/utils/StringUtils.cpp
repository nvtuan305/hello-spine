#include <string.h>

char *getString(const char *str) {
    if (str == NULL)
        return NULL;

    size_t length = strlen(str);
    char *result = new char[length + 1];
    strncpy(result, str, length);
    result[length] = '\0';

    return result;
}
