DZIKI ZACHÓD — symulacja pojedynków (MPI + pthreads)
=====================================================

Projekt z przedmiotu systemy rozproszone. Procesy konkurują o miejsca w salonie
(sekcja krytyczna z pojemnością S), dobierają się w pary i toczą pojedynki.

Algorytmy:

- zegary Lamporta
- wzajemne wykluczanie z pojemnością S (wariant Ricart-Agrawala dla k-mutex)
- dobór w pary oparty o ``ready_list`` sortowaną po ``(ts, pid)``

Kompilacja i uruchomienie
-------------------------

::

    make            # kompiluje
    make run        # uruchamia mpirun -np 8
    make clean      # sprząta

Parametr ``S`` (pojemność salonu) można podać jako argv[1], domyślnie 2.

Struktura
---------

- ``main.c/.h`` — inicjalizacja MPI, zmienne globalne, makra ``println``/``debug``
- ``watek_glowny.c`` — pętla stanów procesu
- ``watek_komunikacyjny.c`` — odbiór wiadomości MPI
- ``salon.c`` — wejście do salonu (sekcja krytyczna)
- ``parowanie.c`` — dobór w pary do pojedynku
- ``util.c`` — typ pakietu MPI, ``sendPacket``, zarządzanie stanem
