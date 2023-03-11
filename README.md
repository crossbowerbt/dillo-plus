# Overview
[![screenshot](https://raw.githubusercontent.com/w00fpack/dilloNG/main/screenshots/main.jpg)](https://raw.githubusercontent.com/w00fpack/dilloNG/main/screenshots/main.jpg)

Dillo is a lightweight (about 10MB) Linux web browser that does not use Javascript..  This DilloNG is a port of the official Dillo that is located at https://www.dillo.org.  It holds new concepts that will hopefully make it into the official releases.  This repository will also help with CI/CD automation.

Lightweight browsers, such as Dillo, are beneficial for loading websites quickly.  Also, lightweight web browsers can work on older hardware where common browser would take up too many resources, that could freeze up your computer.

Because javascript is not used, extra bloat is not needed to be downloaded.  If you have a slow internet connection, a web page that might have taken 3MB to download might only be 400kB with DilloNG.

Finally, your online experience can be more secure with DilloNG.  Not only does it not use Javascript, rules can also be defined per website domain.  These rules can block connecting to certain domains, block ads and trackers, and require sites to use encryption.

Note that not websites will work correctly without Javascript, so be forewarned.

# Browser Features

Here is a screenshot of the Speed Dial page.  DilloNG also comes with an extensive list of bookmarks that showcase some websites that work without Javascript.
[![screenshot](https://raw.githubusercontent.com/w00fpack/dilloNG/main/screenshots/speeddial.jpg)](https://raw.githubusercontent.com/w00fpack/dilloNG/main/screenshots/speeddial.jpg)

DilloNG does not play media, such as audio and video, in the browser.  Instead you can run media using your preferred desktop media player.  The benefit is that playback will  usually be more streamlined, without jitter.  Your media player might also have options, such as video caching of live streams, recording, streaming and downconverting.
[![screenshot](https://raw.githubusercontent.com/w00fpack/dilloNG/main/screenshots/media_playing.png)](https://raw.githubusercontent.com/w00fpack/dilloNG/main/media_playing.png))

# Installation

## Pre-built Releases

Please check under releases

## Requirements

Please see https://www.dillo.org for requirement details.
*FLTK 1.3
* mbedtls

### Optional
* wget - for downloads

## INSTALLATION AND COMPILING

There is an INSTALL file in this repository that might be of value.  You can also visit https://www.dillo.org for installation details details.

./autogen.sh
./configure --prefix=/usr --sysconfdir=/etc --enable-ssl --with-ca-certs-dir=/etc/ssl/certs
make
make install

# Quickstart

1. Obtain a release for your OS distribution, or compile Dillo from this repository's source code
2. Install the application, and Click on the "DilloNG" menu option.
3.  Browse the internet the way you normally would

# Contributing

If you can use Mercurial, you can use the official code repository at htps://dillo.org.   If you would like to use this GIT repository to submit code changes, please open pull requests against [the GitHub repository](https://github.com/w00fpack/dilloNG/edit/master/README.md). Patches submitted in issues, email, or elsewhere will likely be ignored. Here's some general guidelines when submitting PRs:

 * In your pull request, please:
   * Describe the changes, why they were necessary, etc
   * Describe how the changes affect existing behaviour
   * Describe how you tested and validated your changes
   * Include any relevant screenshots/evidence demonstrating that the changes work and have been tested

[wiki]: https://github.com/w00fpack/dilloNG/wiki
