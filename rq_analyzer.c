/* Ride Quality Measurement Using Accelerometer Sensor */

#define _GNU_SOURCE					// for function getline()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define LINEMAX 150
#define MAX(a, b) ( (a > b)? a : b )
#define SAMPMAX	(int) 200e+3		// initial number of samples
#define PRINT_ERROR { perror(NULL); exit(EXIT_FAILURE);}

int nsamples;		// number of samples
int totalTime;		// total time of samples in msec
float avgTime; 		// average time of samples in msec

char **lineptr;
int nlines;

// errors function
void
printAndExit(char *s)
{
	puts(s);
	exit(EXIT_FAILURE);
}

// read lines until reach EOF
// or error occurs
void
readLines(FILE *fp)
{
	char *line;
	size_t bufsize = 0;				// getline will set the required value
	int alloc = 100e+3;
	lineptr = malloc(alloc * sizeof(char *));
	if( lineptr == NULL )
		PRINT_ERROR;
	
	for( nlines = 0; ; nlines++ )
	{
		line = NULL;		// getline will allocate for it
		if( getline(&line, &bufsize, fp) == -1 )  
			break;			// if EOF reached or error occured

		// reallocate if needed:
		if( nlines > alloc )
		{
			alloc *= 2;
			lineptr = realloc(lineptr, alloc * sizeof(char *));
			if( lineptr == NULL )
				PRINT_ERROR;
		}

		// store the line:
		lineptr[nlines] = line;
	}
}

// read a file
void
readFile(void)
{
	char fileName[25];

	puts("Enter name of file");
	fgets(fileName, 25, stdin);
	fileName[strlen(fileName)-1] = '\0'; 		//deleting newline char 
	
	FILE *fp = fopen(fileName, "r");
	if( fp == NULL )
		PRINT_ERROR;

	readLines(fp);
	
	if( fclose(fp) == EOF )
		PRINT_ERROR;
}

// checks if line is comment or empty:
_Bool isComment(char *s)
{
	if( *s == '#' || isspace(*s) )
		return 1;
	else
		return 0;
}

// this function analyzes the input file into tokens
// and generates the output arrays(xval, yval, zval, tval)
void
analyzeFile(float *xp, float *yp, float *zp, int *tp)
{	
	puts("Analyzing File...");
	char **lnptr = lineptr;			// sampling pointer array

	// skip comments and empty lines:
	while( isComment(*lnptr) ) 
	{
		lnptr++;
	}

	int samp = 0;	
	for(; !isComment(*lnptr) ; lnptr++, samp++ )
		// data format in file: "X Y Z time_from_previous_sample(ms)"
		sscanf(*lnptr, "%f%f%f%d", xp++, yp++, zp++, tp++);

	free(lineptr);

	nsamples = samp;
}

// sets total and average time in msec
void
calcTime(int *tp)
{
	int i = nsamples;
	int tot = 0;
	for(; i-- > 0; tp++ )
		tot += *tp;

	totalTime = tot;		// total time in msec
	avgTime = (float) totalTime / nsamples;
}

// deletes some samples at beginning and end of data.
void
dismissSeconds(float **pxval, float **pyval, float **pzval, int **ptval)
{
	char c;
	puts("Dismiss 5 seconds at beginning and end? (y/n)?");
	if( (c = getchar()) == EOF )
		printAndExit("error: input error!");
	else if( c == 'n' || c == 'N' )
		return;
	else if( c != 'y' && c != 'Y' )
		printAndExit("error: undefined input!");

	int t = 5 * 1000;		// time to skip in msec

	// skip begining:
	int i = t;
	while( i > 0 )
	{
		i -= **ptval;
		nsamples--, ++*pxval, ++*pyval, ++*pzval, ++*ptval;
	}

	// skip ending:
	int *tp = *ptval;
	i = t;
	while( i > 0 )
	{
		i -= *(tp + nsamples);
		nsamples--;
	}

	// recalculate time:
	calcTime(*ptval);
}

void
constructVectors(float* vp, float *xp, float* yp, float *zp)
{
	int i;
	for( i = 0; i <= nsamples; i++ )
	{
		// v = cuberoot( (x^3) + (y^3) + (z^3) )
		*vp = cbrtf(pow(*xp, 3) + pow(*yp, 3) + pow(*zp, 3));
		vp++, xp++, yp++, zp++;
	}
}

// old function
float
findVal(float *val, float mode)
{
	int i = nsamples;

	if( !mode )
	{
		float valmax = *val;
		for(; i-- > 0; val++)
			valmax = MAX(valmax, fabsf(*val));

		return valmax;
	}
	else
	{
		for(; i-- > 0; val++)
			if( fabsf(*val) >= mode )
				printf("%d: %g\n"
					, nsamples-i, fabsf(*val));
	}
	return 0;
}

// old function
void
printMax(float *val, char *s)
{
	printf("%s = %g\n", s, findVal(val, 0));
}

int
findVals(float *xp, float lim)
{
	int i = nsamples;
	int vals = 0;

	for(; i-- > 0; xp++)
		if( fabsf(*xp) >= lim )
			vals++;

	return vals;
}
// vibration values struct:
struct VibVals {	
	float *level;		// 20 levels starting from 1
	int *number;		// number of vibrations above certain level
	float *percent;		// percent of vibrations above certain level 
	float *avg;			// average time of vibrations above certain level
};

// this function checks if the report applies to some rules
// and sets the ride quality according to these rules.
void
calcRideQuality(struct VibVals *vibp, float maxLevel)
{
	int quality = 0;
	// rules:
	
	// 1- vibration max rule: this rule checks if maximum 
		// vibration of the object is below a certain level.
	float vibMaxRule[11] = {0, 2.0, 3.0, 4.0, 6.0, 7.0
		, 9.0, 10.0, 11.0, 13.0, 15.0};
	
	// 2- percent rule:  this rule checks if vibrations of the 
		// object under certain level is below a certain percent.
	float percentRule[11] = {0, 0.02, 0.2, 2.0, 5.0, 9.0
		, 15.0, 25.0, 40.0, 50.0, 60.0};

	int stars = 0;	// number of stars in rating (of 10)
	int ri;			// rule index
	for(ri = 1; ri < 11; ri++)
		if( vibMaxRule[ri] >= maxLevel )
		{
			int j;
			for(j = ri-1; j >= 0; j--)
				// if one rule not apply, return to parent for
				if( vibp->percent[ri-j] > percentRule[j+1] )	
					break;

			// if all rules apply, set rating and exit loop.
			if( j < 0 )		
			{
				stars = 11 - ri;
				break;
			}
		}
	// starStr holds some '*'s equal to the number of stars
	char *starStr = malloc(11);
	char *p = starStr;
	while( p < starStr + stars )
	{
		*p++ = '*';
		*p = '\0';
	}

	printf("\n%s Your Ride Quality is %d of 10! %s\n", starStr, stars, starStr);
	free(starStr);	
}

void
generateReport(float *pval)	
{
	puts("Generating Report...\n");
	struct VibVals* vibp = malloc(sizeof(struct VibVals *));
	vibp->level = malloc(21 * sizeof(float));
	vibp->number = malloc(21 * sizeof(int));
	vibp->percent = malloc(21 * sizeof(float));
	vibp->avg = malloc(21 * sizeof(int));

	int nVals;
	int a, max = 20;
	for( a = 1; a <= max; a++ )
	{
		nVals = findVals(pval, (float) a);
		if( nVals )
		{
			vibp->level[a] = (float) a;
			vibp->number[a] = nVals;
			vibp->percent[a] = (float) ((100 * nVals) / (float) nsamples);
			vibp->avg[a] = (float) (nVals * avgTime) /1000;
			int printed = printf(
				"Vibrations more than %2.0f m/sec^2 represents %.3f%%\n"
				//", avg time %.3f sec\n"
				, vibp->level[a]
				, vibp->percent[a]
				);
			while( printed-- > 0 )
				putchar('-');

			putchar('\n');
		}
		else
		{ 
			calcRideQuality(vibp, (float) a);
			break;
		}
	}

	puts("\nAdditional info:");
	printf("Number of samples tested = %d\n", nsamples);
	printf("Total time of samples= %.2f sec\n", (float) totalTime/1000);
	printf("Average sample time = %.3f msec\n", avgTime);
}

int main(void)
{
	puts("--* Welcome to Ride Quality Analyzer! *--\n");

	// read the file
	readFile();

	float *xval, *yval, *zval;
	int *tval;

	xval = malloc(nlines * sizeof(float));
	if( xval == NULL )
		{ PRINT_ERROR }
	yval = malloc(nlines * sizeof(float));
	if( yval == NULL )
		{ PRINT_ERROR }
	zval = malloc(nlines * sizeof(float));
	if( zval == NULL )
		{ PRINT_ERROR }
	tval = malloc(nlines * sizeof(int));
	if( tval == NULL )
		{ PRINT_ERROR }

	// extract information:
	analyzeFile(xval, yval, zval, tval);

	// calculate total and average time in msec:
	calcTime(tval);

	// skip boundary seconds:
	dismissSeconds(&xval, &yval, &zval, &tval);

	// construct vector data:
	float *vval = malloc(nsamples * sizeof(float));
	if( vval == NULL )
		{PRINT_ERROR}
	else
		constructVectors(vval, xval, yval, zval);

	// analyze data and output a report & calculate ride quality
	generateReport(vval);

	if(0)
	{
		int i;
		for( i = 0; i <= nsamples; i++)
			printf("%d: %f, %f, %f, %d\n", i, *xval++, *yval++, *zval++, *tval++);
	}

	printf("\n");
	return 0;
}