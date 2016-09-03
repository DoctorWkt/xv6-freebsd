/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)setlocale.c	8.1 (Berkeley) 7/4/93";
#endif /* LIBC_SCCS and not lint */

#include <limits.h>
#include <locale.h>
#include <rune.h>
#include <stdlib.h>
#include <string.h>

/*
 * Category names for getenv()
 */
static char *categories[_LC_LAST] = {
    "LC_ALL",
    "LC_COLLATE",
    "LC_CTYPE",
    "LC_MONETARY",
    "LC_NUMERIC",
    "LC_TIME",
};

/*
 * Current locales for each category
 */
static char current_categories[_LC_LAST][32] = {
    "C",
    "C",
    "C",
    "C",
    "C",
    "C",
};

/*
 * The locales we are going to try and load
 */
static char new_categories[_LC_LAST][32];

static char current_locale_string[_LC_LAST * 33];
static char *PathLocale;

static char	*currentlocale __P((void));
static char	*loadlocale __P((int));

char *
setlocale(category, locale)
	int category;
	const char *locale;
{
	int found, i, len;
	char *env, *r;

	if (!PathLocale && !(PathLocale = getenv("PATH_LOCALE")))
		PathLocale = _PATH_LOCALE;

	if (category < 0 || category >= _LC_LAST)
		return (NULL);

	if (!locale)
		return (category ?
		    current_categories[category] : currentlocale());

	/*
	 * Default to the current locale for everything.
	 */
	for (i = 1; i < _LC_LAST; ++i)
		(void)strcpy(new_categories[i], current_categories[i]);

	/*
	 * Now go fill up new_categories from the locale argument
	 */
	if (!*locale) {
		env = getenv(categories[category]);

		if (!env)
			env = getenv(categories[0]);

		if (!env)
			env = getenv("LANG");

		if (!env)
			env = "C";

		(void) strncpy(new_categories[category], env, 31);
		new_categories[category][31] = 0;
		if (!category) {
			for (i = 1; i < _LC_LAST; ++i) {
				if (!(env = getenv(categories[i])))
					env = new_categories[0];
				(void)strncpy(new_categories[i], env, 31);
				new_categories[i][31] = 0;
			}
		}
	} else if (category)  {
		(void)strncpy(new_categories[category], locale, 31);
		new_categories[category][31] = 0;
	} else {
		if ((r = strchr(locale, '/')) == 0) {
			for (i = 1; i < _LC_LAST; ++i) {
				(void)strncpy(new_categories[i], locale, 31);
				new_categories[i][31] = 0;
			}
		} else {
			for (i = 1; r[1] == '/'; ++r);
			if (!r[1])
				return (NULL);	/* Hmm, just slashes... */
			do {
				len = r - locale > 31 ? 31 : r - locale;
				(void)strncpy(new_categories[i++], locale, len);
				new_categories[i++][len] = 0;
				locale = r;
				while (*locale == '/')
				    ++locale;
				while (*++r && *r != '/');
			} while (*locale);
			while (i < _LC_LAST)
				(void)strcpy(new_categories[i],
				    new_categories[i-1]);
		}
	}

	if (category)
		return (loadlocale(category));

	found = 0;
	for (i = 1; i < _LC_LAST; ++i)
		if (loadlocale(i) != NULL)
			found = 1;
	if (found)
	    return (currentlocale());
	return (NULL);
}

static char *
currentlocale()
{
	int i;

	(void)strcpy(current_locale_string, current_categories[1]);

	for (i = 2; i < _LC_LAST; ++i)
		if (strcmp(current_categories[1], current_categories[i])) {
			(void)snprintf(current_locale_string,
			    sizeof(current_locale_string), "%s/%s/%s/%s/%s",
			    current_categories[1], current_categories[2],
			    current_categories[3], current_categories[4],
			    current_categories[5]);
			break;
		}
	return (current_locale_string);
}

static char *
loadlocale(category)
	int category;
{
	char name[PATH_MAX];

	if (strcmp(new_categories[category],
	    current_categories[category]) == 0)
		return (current_categories[category]);

	if (category == LC_CTYPE) {
		if (setrunelocale(new_categories[LC_CTYPE]))
			return (NULL);
		(void)strcpy(current_categories[LC_CTYPE],
		    new_categories[LC_CTYPE]);
		return (current_categories[LC_CTYPE]);
	}

	if (!strcmp(new_categories[category], "C") ||
		!strcmp(new_categories[category], "POSIX")) {

		/*
		 * Some day this will need to reset the locale to the default
		 * C locale.  Since we have no way to change them as of yet,
		 * there is no need to reset them.
		 */
		(void)strcpy(current_categories[category],
		    new_categories[category]);
		return (current_categories[category]);
	}

	/*
	 * Some day we will actually look at this file.
	 */
	(void)snprintf(name, sizeof(name), "%s/%s/%s",
	    PathLocale, new_categories[category], categories[category]);

	switch (category) {
		case LC_COLLATE:
		case LC_MONETARY:
		case LC_NUMERIC:
		case LC_TIME:
			return (NULL);
	}
}
