# Kesara

Kesara is a Sinhala phonetic (Singlish-style) keyboard for Linux. Type Latin
letters and they convert to Sinhala Unicode as you go: `ka` → ක, `th` → ත්,
`kra` → ක්‍ර.

- **IBus** uses the original m17n table (`m17n:si:kesara`)
- **Fcitx 5** (KDE Plasma) and **Fcitx 4** use a native engine that implements
  the same state machine, so conversion matches the m17n table

## Packages

Install **one** package for the input framework you actually use. Built packages
are attached to [GitHub Releases](https://github.com/SuhasDissa/kesara-keyboard/releases)
— this repository contains source only.

| Package | Framework | Typical desktop |
| --- | --- | --- |
| `ibus-kesara` | IBus + m17n | GNOME, Cinnamon, others |
| `fcitx5-kesara` | Fcitx 5 | KDE Plasma 5/6, many Wayland setups |
| `fcitx-kesara` | Fcitx 4 | Older Fcitx 4 desktops |

### Arch Linux

```bash
sudo pacman -U ibus-kesara-VERSION-any.pkg.tar.zst      # IBus
sudo pacman -U fcitx5-kesara-VERSION-x86_64.pkg.tar.zst # Fcitx 5
```

### Debian / Ubuntu

```bash
sudo apt install ./ibus-kesara_VERSION_all.deb          # IBus
sudo apt install ./fcitx5-kesara_VERSION_amd64.deb      # Fcitx 5
sudo apt install ./fcitx-kesara_VERSION_amd64.deb       # Fcitx 4
```

`ibus-kesara` depends on `ibus-m17n` and `m17n-db`. The Fcitx packages depend
on `fcitx5` or `fcitx` respectively.

### Fedora / RHEL

```bash
sudo dnf install ./ibus-kesara-VERSION.noarch.rpm
sudo dnf install ./fcitx5-kesara-VERSION.x86_64.rpm
sudo dnf install ./fcitx-kesara-VERSION.x86_64.rpm
```

## Enable the input method

Log out and back in (or restart the input daemon) after installing.

**GNOME / IBus:** Settings → Keyboard → Input Sources → Add → Sinhala →
**Kesara** (`m17n:si:kesara`). Switch with Super+Space.

**KDE / Fcitx 5:** System Settings → Input Method → Configure → Add → **Kesara**.
Switch with Ctrl+Space (default).

**Fcitx 4:** Right-click the tray icon → Configure → Input Method → Add →
**Kesara**.

## Build from source

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr \
  -DBUILD_IBUS=ON -DBUILD_FCITX5=AUTO -DBUILD_FCITX4=AUTO
cmake --build build
ctest --test-dir build --output-on-failure
sudo cmake --install build
```

- `BUILD_FCITX5` / `BUILD_FCITX4`: `ON`, `OFF`, or `AUTO` (default). `AUTO`
  builds the addon when the corresponding development package is present.
- IBus-only: `sudo cmake --install build --component ibus`

Needs: CMake 3.16+, a C compiler (and C++17 for Fcitx 5), plus
`extra-cmake-modules` and `libfcitx5core-dev` / `fcitx-dev` for the native
addons.

## Phonetic chart

Consonants are typed with an inherent virama; a following vowel removes it.

| Latin | Sinhala | Latin | Sinhala |
| --- | --- | --- | --- |
| k / ka | ක් / ක | kh | ඛ |
| g | ග | gh | ඝ |
| ch | ච | j | ජ |
| t | ට | T | ඨ |
| th | ත | dh | ද |
| d | ඩ | n | න |
| p | ප | b | බ |
| m | ම | y | ය |
| r | ර | l | ල |
| w, v | ව | sh | ශ |
| s | ස | S | ෂ |
| h | හ | L | ළ |
| f | ෆ | NG | ක්‍ෂ |

Vowels (independent at the start of a syllable; dependent after a consonant):

| Latin | Independent | Dependent |
| --- | --- | --- |
| a | අ | (inherent) |
| aa | ආ | ා |
| A | ඇ | ැ |
| AA, Aa | ඈ | ෑ |
| i / ii | ඉ / ඊ | ි / ී |
| u / uu | උ / ඌ | ු / ූ |
| e / ee | එ / ඒ | ෙ / ේ |
| o / oo | ඔ / ඕ | ො / ෝ |
| ou | ඖ | ෞ |
| x | ං | |

After a consonant, `r` is rakaaranshaya (ක්‍ර) and `y` is yansaya (ක්‍ය).
Type the vowel first if you want a full ර / ය letter (`kara` → කර).

## Releasing

```bash
git tag v1.0.0
git push origin v1.0.0
```

Pushing a `v*` tag runs GitHub Actions: engine tests, then Arch / Debian /
Fedora packages plus a source tarball, attached to a GitHub Release with
SHA256 checksums.

## Credits

The transliteration scheme and much of the original `.mim` table come from
**madura.x86** (Realtime Singlish / `singlish.mim`) and **Harshula Jayasuriya**.

Linux packaging, the shared conversion engine, and the native Fcitx addons:
[Suhas Dissanayake](https://suhasdissa.top).

## License

[GNU General Public License v3.0 or later](LICENSE).
