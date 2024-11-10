# ePaper Badge

A project to use a 122x250 ePaper display as a digital badge.

## Overview

This project displays a digital badge on an ESP32-powered 2.13" ePaper display. It shows fields for name, title, and email, along with a scannable QR code. Data can be configured and transferred via the Chronos app, and stored on the ESP32 using NVS (non-volatile storage).

## Features

- Displays user information (name, title, email) on the badge
- Generates and displays a QR code based on user-provided data
- Configurable via Chronos app for easy setup
- Stores data on ESP32 NVS storage for persistent display after reboot

## Requirements

1. ESP32 2.13" ePaper Display (122x250 resolution)
   - [`Elecrow ePaper Display`](https://www.elecrow.com/crowpanel-esp32-2-13-e-paper-hmi-display-with-122-250-resolution-black-white-color-driven-by-spi-interface.html)
2. Chronos App (for configuration and data transfer)
   - Set up fields on the QR page:
     - `name` -> WeChat
     - `title` -> Facebook
     - `email` -> QQ
     - `link` -> Twitter
