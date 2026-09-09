# Plan : RTC externe DS3231 (I2C) pour simple_repeater

## Objectif

Utiliser une RTC externe DS3231 (adresse I2C 0x68) comme horloge principale du
repeteur (cible : NRF52), avec repli silencieux sur l'horloge interne existante
si la puce est absente, non alimentee ou defaillante.

Contraintes :
- Diff minimal sur le code existant (fork amont qui bouge beaucoup).
- Maximum de code dans de nouveaux fichiers independants prefixe `CRT_`.
- Pas de build realise par l'IA : compilation/flash par l'utilisateur.

## Etat valide

Plan valide par l'utilisateur le 2026-08-27 (points 1 a 4 OK).
Le developpement ne commence pas encore.

## Fichiers NOUVEAUX (prefixe `CRT_`)

### 1. `examples/simple_repeater/CRT_RTCClock.h`
### 2. `examples/simple_repeater/CRT_RTCClock.cpp`

Classe `CRT_RTCClock : public mesh::RTCClock` :
- Utilise `RTC_DS3231` de RTClib (deja une dependance du projet, aucun `lib_deps` a ajouter).
- `begin(TwoWire& wire)` : `wire.begin()` + detection/ping de la DS3231 a 0x68.
- `getCurrentTime()` : renvoie l'heure DS3231 **si la puce repond ET que l'heure
  est plausible (>= 2024-01-01, macro reglable)** ; sinon repli silencieux sur le
  global `rtc_clock` existant.
  Garde-fou : evite que les timestamps "annee 2000" d'une DS3231 neuve cassent
  l'anti-rejeu du mesh.
- `setCurrentTime()` : ecrit dans la DS3231 (le CLI `clock` existant passera par la).
- `tick()` : met a jour le fallback (indispensable pour `VolatileRTCClock`).
- `isPresent()` / diagnostics (logs en `MESH_DEBUG`).
- Tout le code est encapsule dans `#ifdef CRT_RTC_ENABLED` : sans le flag, le
  `.cpp` compile a vide (il est compile dans tous les env repeater car le dossier
  est inclus par les filters) et n'a aucun effet.

## Fichiers MODIFIES (diff minimal)

### 3. `examples/simple_repeater/main.cpp` — blocs `#ifdef CRT_RTC_ENABLED`

- global `CRT_RTCClock crt_clock(rtc_clock);` (fallback = horloge carte existante)
- construction : `MyMesh the_mesh(..., crt_clock, tables)` au lieu de `rtc_clock`
- `crt_clock.begin(Wire);` dans `setup()`
- `crt_clock.tick();` dans `loop()` (remplace `rtc_clock.tick()`)

### 4. `examples/simple_repeater/MyMesh.cpp` — 5 remplacements

`rtc_clock.getCurrentTime()` -> `getRTCClock()->getCurrentTime()` :
- ligne 148 (`handleAnonRegionsReq`)
- ligne 166 (`handleAnonOwnerReq`)
- ligne 185 (`handleAnonClockReq`)
- ligne 792 (`onControlDataRecv`, discover_limiter)
- ligne 841 (`onControlDataRecv`, putNeighbour)

Refactor pur : en stock, `getRTCClock()` pointe sur le meme objet que le global
`rtc_clock`, donc comportement identique. Quand la CRT est active, tous les
timestamps (limiteurs anti-spam, neighbors, discovery) passent par la DS3231.

## Configuration (aucun fichier de build modifie)

Activer au moment de l'utilisation : ajouter `-D CRT_RTC_ENABLED` dans l'env de
build (ex. `platformio.local.ini` ou la section `[env:...]` NRF52 de la carte).
Sans le flag -> firmware 100 % identique au stock.

## Verification prevue (par l'utilisateur)

1. Compiler l'env NRF52 avec `-D CRT_RTC_ENABLED` et `MESH_DEBUG`.
2. Boot : log de detection DS3231 ; retrait de la carte -> repli silencieux.
3. CLI `clock <timestamp>` -> ecrit dans la DS3231 ; reboot -> heure conservee.

## Decisions validees

1. Seuil de plausibilite `>= 2024-01-01` (modifiable par macro) — OK
2. Ne pas toucher aux `variants/*` (target.cpp/cible reste la source du global `rtc_clock`) — OK
3. Les 5 remplacements dans `MyMesh.cpp` — OK
4. Pas d'env PlatformIO dedie dans un `variants/*/platformio.ini` (activation via `platformio.local.ini`) — OK

## Notes techniques

- La DS3231 est deja sondee par `AutoDiscoverRTCClock` sur la plupart des cartes
  (heltec, meshtiny, etc.) : la couche CRT ajoute un controle explicite de
  l'ordre de priorite, un garde-fou anti-rejeu si la puce n'est pas reglee, et
  fonctionne meme sur les cartes dont `rtc_clock` est un simple `VolatileRTCClock`.
- Le CLI `clock` de `CommonCLI` passe par le pointeur RTC fourni au constructeur :
  _cli, le bridge et Mesh reçoivent tous `crt_clock` via le constructeur de `MyMesh`.