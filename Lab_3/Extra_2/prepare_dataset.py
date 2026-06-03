import argparse
import io
import sys
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageEnhance, ImageFilter, ImageOps


VALID_EXTENSIONS = {'.jpg', '.jpeg', '.png', '.bmp', '.webp'}
OUTPUT_SIZE = 128
DISPLAY_SIZE = 384  # 128 * 3, upscaled for visibility
TAG_LABELS = {0: "Not present", 1: "Left", 2: "Center", 3: "Right"}


def center_crop_square(pil_img):
    w, h = pil_img.size
    side = min(w, h)
    left = (w - side) // 2
    top = (h - side) // 2
    return pil_img.crop((left, top, left + side, top + side))


def load_image(pil_img):
    """EXIF-correct, grayscale, center crop, resize to 128x128."""
    img = ImageOps.exif_transpose(pil_img).convert('L')
    img = center_crop_square(img)
    return np.array(img.resize((OUTPUT_SIZE, OUTPUT_SIZE), Image.LANCZOS), dtype=np.uint8)


def degrade_to_esp(arr):
    """Simulate OV2640: contrast reduction, blur, JPEG artifacts, sensor noise."""
    img = Image.fromarray(arr)
    img = ImageEnhance.Contrast(img).enhance(0.75)
    img = img.filter(ImageFilter.GaussianBlur(radius=0.6))

    buf = io.BytesIO()
    img.save(buf, format='JPEG', quality=60)
    buf.seek(0)
    img = Image.open(buf).copy()

    arr = np.array(img, dtype=np.float32)
    arr += np.random.normal(0, 7, arr.shape)
    return np.clip(arr, 0, 255).astype(np.uint8)


def build_display(arr, idx, total):
    display = cv2.resize(arr, (DISPLAY_SIZE, DISPLAY_SIZE), interpolation=cv2.INTER_NEAREST)
    display = cv2.cvtColor(display, cv2.COLOR_GRAY2BGR)

    third = DISPLAY_SIZE // 3
    color = (0, 220, 0)
    cv2.line(display, (third, 0), (third, DISPLAY_SIZE), color, 2)
    cv2.line(display, (2 * third, 0), (2 * third, DISPLAY_SIZE), color, 2)

    for label, x in [("L", third // 2), ("C", third + third // 2), ("R", 2 * third + third // 2)]:
        cv2.putText(display, label, (x - 8, 24), cv2.FONT_HERSHEY_SIMPLEX, 0.75, color, 2)

    cv2.putText(display, f"{idx}/{total}", (DISPLAY_SIZE - 68, 24),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (190, 190, 190), 1)

    hint = "0=None  1=Left  2=Center  3=Right  q=Skip  d=Delete  Esc=Save&Quit"
    cv2.putText(display, hint, (4, DISPLAY_SIZE - 8),
                cv2.FONT_HERSHEY_SIMPLEX, 0.34, (190, 190, 190), 1)

    return display


def save_results(tagged, output_dir):
    output_dir.mkdir(exist_ok=True)
    tag_lines = []
    for i, (arr, tag) in enumerate(tagged):
        name = f"{i:03d}.png"
        cv2.imwrite(str(output_dir / name), arr)
        tag_lines.append(f"{name},{tag}")

    (output_dir / 'tags.txt').write_text('\n'.join(tag_lines) + '\n')
    print(f"\nSaved {len(tagged)} images to {output_dir}")


def main():
    parser = argparse.ArgumentParser(
        description='Prepare and tag images for ESP-CAM identifier detection dataset.')
    parser.add_argument('input_dir', help='Directory containing images')
    mode_group = parser.add_mutually_exclusive_group()
    mode_group.add_argument('-cel', action='store_true',
                            help='Apply ESP-CAM transformations (smartphone images)')
    mode_group.add_argument('-esp', action='store_true',
                            help='No transformation, images already from ESP-CAM (default)')
    args = parser.parse_args()

    transform = args.cel

    input_path = Path(args.input_dir).resolve()
    if not input_path.is_dir():
        sys.exit(f"Error: '{input_path}' is not a directory")

    images = sorted(f for f in input_path.iterdir()
                    if f.suffix.lower() in VALID_EXTENSIONS)
    if not images:
        sys.exit(f"No images found (supported: {', '.join(VALID_EXTENSIONS)})")

    mode_label = "ESP-CAM simulation (-cel)" if transform else "no transformation (default/-esp)"
    print(f"Found {len(images)} images — mode: {mode_label}")
    print("Controls: 0=None  1=Left  2=Center  3=Right  q=Skip  d=Delete  Esc=Save&Quit")

    tagged = []
    cv2.namedWindow('Tagger', cv2.WINDOW_NORMAL)
    cv2.resizeWindow('Tagger', DISPLAY_SIZE, DISPLAY_SIZE)

    for i, path in enumerate(images):
        try:
            arr = load_image(Image.open(path))
            if transform:
                arr = degrade_to_esp(arr)
        except Exception as e:
            print(f"[{i+1}/{len(images)}] Could not process {path.name}: {e} — skipping")
            continue

        cv2.imshow('Tagger', build_display(arr, i + 1, len(images)))
        cv2.setWindowTitle('Tagger', f'Tagger — {path.name}')

        while True:
            key = cv2.waitKey(0) & 0xFF
            if chr(key) in '0123':
                tag = int(chr(key))
                tagged.append((arr, tag))
                print(f"[{i+1}/{len(images)}] {path.name} → {tag} ({TAG_LABELS[tag]})")
                break
            elif key == ord('q'):
                print(f"[{i+1}/{len(images)}] {path.name} → skipped")
                break
            elif key == ord('d'):
                print(f"[{i+1}/{len(images)}] {path.name} → deleted")
                break
            elif key == 27:  # Esc
                cv2.destroyAllWindows()
                if tagged:
                    save_results(tagged, input_path / 'new')
                sys.exit(0)

    cv2.destroyAllWindows()
    if tagged:
        save_results(tagged, input_path / 'new')
    else:
        print("No images were tagged.")


if __name__ == '__main__':
    main()
