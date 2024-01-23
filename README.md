# Protocolul BitTorrent

## 1. Descriere

Acest program implementează o rețea peer-to-peer (P2P) pentru partajarea fișierelor. Utilizând interfețele de programare aplicativă (API) din MPI (Message Passing Interface), programul permite transferul eficient de date între nodurile rețelei. Structura programului este împărțită în mai multe module principale, fiecare având responsabilități specifice:

* `peer.cpp` și `peer.h`
    * gestionează funcționalitățile de bază ale unui peer în rețea, inclusiv citirea fișierelor de intrare, inițializarea clientului și coordonarea activităților generale ale peer-ului.

* `tracker.cpp` și `tracker.h`
    * tracker-ul este centrul de coordonare al rețelei, gestionând informațiile despre fișierele disponibile nodurile ce le detin.
    * se ocupă de inițializarea tracker-ului și de gestionarea cererilor de la peers.

* `download.cpp` și `download.h`
    * se ocupă de procesul de descărcare a fișierelor, inclusiv solicitarea și primirea informațiilor de la tracker și gestionarea descărcărilor de la alți peers.

* `update.cpp` și `update.h`
    * responsabile pentru actualizarea și transferul de date între peers, aceste fișiere permit schimbul de segmente de fișiere

## 2. Inițializare

La inițializarea programului, fiecare peer își citește configurația dintr-un fișier de intrare specificat. Acest proces include determinarea fișierelor pe care peer-ul le deține și pe care dorește să le descarce.

După citirea datelor, peer-ul se conectează la rețea, inițializând comunicația cu tracker-ul și cu alți peer-uri. Inițializarea include configurarea thread-urilor pentru descărcare și încărcare, precum și stabilirea conexiunilor necesare pentru comunicația prin MPI.

## 3. Update
Nodurile ce detin anumite segmente dintr-un fișier se pot conecta la alte noduri pentru a le trimite acele segmente. Acest proces este gestionat de modulele update.cpp și update.h.

Pe baza informațiilor primite de la tracker, peer-ul inițiază conexiuni cu alte peer-uri și începe transferul de date. Acest proces se desfășoară în paralel cu procesul de descărcare, astfel încât peer-ul să poată descărca și încărca în același timp.

Fisierele unui peer sunt salvate in memorie sub urmatoarea forma:
```
files = {
    "file1": [
        "segment1",
        "segment2"
    ],
    "file2": [
        "segment1",
        "segment2",
        "segment3"
    ],
}
```

## 4. Download
Procesul de descărcare este un aspect cheie al funcționării rețelei P2P. Modulele download.cpp și download.h se ocupă de gestionarea descărcărilor de fișiere de la alți peer-uri.

Acest proces începe prin solicitarea informațiilor de la tracker despre locația segmentelor de fișiere dorite. Pe baza acestor informații, peer-ul inițiază conexiuni cu peer-uri care dețin segmentele necesare și începe descărcarea. Mecanismele de gestionare a swarm-urilor asigură distribuirea eficientă a sarcinilor de descărcare și menținerea unui echilibru între ofertă și cerere în rețea.

Pentru ca tracker-ul sa poata gestiona cererile de download, acesta salveaza informatii despre fiecare segment in parte, si despre ce peer il detine, sub forma:
```
files = {
    "file1": [
        ("segment1", [1, 2, 3]),
        ("segment2", [1, 2, 3])
    ],
    "file2": [
        ("segment1", [1, 3]),
        ("segment2", [1, 2, 3]),
        ("segment3", [2, 3])
    ]
}
```

---