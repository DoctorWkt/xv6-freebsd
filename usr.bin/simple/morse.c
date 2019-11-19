#include <stdio.h>
#include <ctype.h>

char	*dit	= "dit";
char	*daw	= "daw";

extern void beep(int);

char	*digit[] = {
	"-----",
	".----",
	"..---",
	"...--",
	"....-",
	".....",
	"-....",
	"--...",
	"---..",
	"----.",
	0
};

char *alph[] = {
	".-",
	"-...",
	"-.-.",
	"-..",
	".",
	"..-.",
	"--.",
	"....",
	"..",
	".---",
	"-.-",
	".-..",
	"--",
	"-.",
	"---",
	".--.",
	"--.-",
	".-.",
	"...",
	"-",
	"..-",
	"...-",
	".--",
	"-..-",
	"-.--",
	"--..",
	0};

main() {

	register c;

	while ((c = getchar()) != EOF) {
		if (isupper(c))
			c = tolower(c);
		if (isalpha(c))
			print(alph[c-'a']);
		else if (isdigit(c))
			print(digit[c-'0']);
		else if (c == ',')
			print("--..--");
		else if (c == '.')
			print(".-.-.-");
		else if (isspace(c))
			printf(" ...\n");
	}
}

print(s) char *s; {
	char *p;
	for (p = s; *p; p++)
      if (*p == '.') {
			printf(" %s", dit);
            beep(1000);
            sleep(5);
      } else if (*p == '-') {
			printf(" %s", daw);
            beep(1000);
            sleep(2);
      }
    sleep(5);
	printf(",\n");
}
