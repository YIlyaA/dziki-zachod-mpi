#include "main.h"
#include "salon.h"
#include "parowanie.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    changeState(REST);

    while (1) {
        switch (stan) {
            case REST:
                /* TODO */
                sleep(1);
                break;

            case WAIT_SALON:
                /* TODO */
                sleep(1);
                break;

            case IN_SALON:
                /* TODO */
                sleep(1);
                break;

            case WAIT_DUEL:
                pthread_mutex_lock(&state_cond_mut);
                while (stan == WAIT_DUEL)
                    pthread_cond_wait(&duel_cond, &state_cond_mut);
                pthread_mutex_unlock(&state_cond_mut);
                break;

            case IN_DUEL:
                println("Wchodzę do pojedynku z %d", partner_id);
                salon_release();
                sleep(rand() % SEC_IN_STATE + 1);
                if (rank < partner_id) {
                    println("Przegrałem");
                    changeState(HOSPITAL);
                } else {
                    println("Wygrałem");
                    changeState(REST);
                }
                break;

            case HOSPITAL:
                println("W szpitalu");
                sleep(rand() % SEC_IN_STATE + 1);
                changeState(REST);
                break;
        }
    }
}
