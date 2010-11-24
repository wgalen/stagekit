#include <stdio.h>
#include <stdlib.h>
#include "stagekit.h"

int main()
{
    int i;
    unsigned char j;
    printf("Initializing StageKit...\n");
    sk_init(NULL);
    printf("0 - Panic\n1-5 - Strobe\n6 - Set red lights\n7 - Set yellow lights\n8 - Set green lights\n9 - Set blue lights\n10 - Fog on\n11 - Fog off\n");
    while (i!=-1)
    {
        if(scanf("%i", &i)==EOF)
            return;

        switch(i)
        {
            case 0:
                sk_alloff();
                printf("Turned off everything\n");
                break;
            case 1:
                sk_nostrobe();
                printf("Turned off strobe\n");
                break;
            case 2:
                sk_slowstrobe();
                printf("Started a slow strobe\n");
                break;
            case 3:
                sk_medstrobe();
                printf("Started a medium strobe\n");
                break;
            case 4:
                sk_faststrobe();
                printf("Started a fast strobe\n");
                break;
            case 5:
                sk_fasteststrobe();
                printf("Started the fastest strobe\n");
                break;
            case 6:
                printf("Enter a value (in hex) for the red lights:");
                scanf("%x", &j);
                sk_setred(j);
                printf("Sent %x to the red array\n",j);
                break;
            case 7:
                printf("Enter a value (in hex) for the yellow lights:");
                scanf("%x", &j);
                sk_setyellow(j);
                printf("Sent %x to the yellow array\n",j);
                break;
            case 8:
                printf("Enter a value (in hex) for the green lights:");
                scanf("%x", &j);
                sk_setgreen(j);
                printf("Sent %x to the green array\n",j);
                break;
            case 9:
                printf("Enter a value (in hex) for the blue lights:");
                scanf("%x", &j);
                sk_setblue(j);
                printf("Sent %x to the blue array\n",j);
                break;
            case 10:
                sk_fogon();
                printf("Fog engaged\n");
                break;
            case 11:
                sk_fogoff();
                printf("Fog disengaged\n");
                break;
        }
    }
    return 0;
}
