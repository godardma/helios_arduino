# Helios arduino

Le programme arduino et stocké sur l'ordi embarqué à l'adresse : /munu_software/motors, dossier firmware (présent dans ce repo)

## uploader le programme arduino après modification

Dans ce repo (un terminal doit être connecté en ssh sur le helios) : 

```bash
scp -r firmware s100@10.43.20.223:munu_software/motors
# mot de passe : s100
```

Sur un terminal connecté en ssh sur le helios :

```bash
cd munu_software/motors/firmware
make
make upload
```

Si tout se passe bien, l'arduino va redémarrer et les brushless vont "chanter"

## Notes :
la batterie est connectée au port A0 de l'arduino
l'arduino est connecté au port série narval_motors de l'ordi embarqué (ALIAS donné à un des tty/USBx correspondant), débit 115200 bauds

