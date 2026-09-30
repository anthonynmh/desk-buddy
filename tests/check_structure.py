"""Check repository layout and local references without an ESP32 toolchain."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SKETCH = ROOT / 'firmware/desk-buddy'
errors = []

required = ['README.md', 'AGENTS.md', 'docs/setup.md', 'docs/architecture.md',
            'docs/assets/Preview.png', 'licenses/LICENSE-Pip.txt',
            'licenses/LICENSE-Font.txt', 'licenses/LICENSE-Waveshare.txt',
            'firmware/desk-buddy/desk-buddy.ino']
for item in required:
    if not (ROOT / item).is_file():
        errors.append(f'Missing {item}')
for module in ('app', 'companion', 'graphics', 'assets', 'hardware'):
    if not (SKETCH / 'src' / module).is_dir():
        errors.append(f'Missing module {module}')

legacy = 'Pip' + 'Companion'
for path in ROOT.rglob('*'):
    if any(part in ('.git', 'build', 'work', '__pycache__') for part in path.parts):
        continue
    if legacy.lower() in path.name.lower():
        errors.append(f'Legacy path: {path.relative_to(ROOT)}')
    if path.suffix not in ('.h', '.cpp', '.ino', '.md'):
        continue
    text = path.read_text(encoding='utf-8')
    if legacy.lower() in text.lower():
        errors.append(f'Legacy reference: {path.relative_to(ROOT)}')
    for include in re.findall(r'^\s*#include\s+"([^"]+)"', text, re.M):
        # Arduino and ESP-IDF headers are supplied by the board package.
        if include.startswith(('esp_', 'driver/', 'freertos/')) or include in ('Arduino.h', 'sdkconfig.h'):
            continue
        if not (path.parent / include).is_file():
            errors.append(f'Unresolved include {include} in {path.relative_to(ROOT)}')
    if path.suffix == '.md':
        for link in re.findall(r'\]\(([^)]+)\)', text):
            if '://' in link or link.startswith('#'):
                continue
            if not (path.parent / link.split('#')[0]).exists():
                errors.append(f'Broken link {link} in {path.relative_to(ROOT)}')

assert not errors, '\n'.join(errors)
print('Structure, sketch naming, local includes, documentation links, and project rename passed.')
