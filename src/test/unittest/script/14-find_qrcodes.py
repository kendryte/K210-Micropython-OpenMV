def unittest(data_path, temp_path):
    import image
    img = image.Image("unittest/data/qrcode.pgm", copy_to_fb=True)
    codes = img.find_qrcodes()
    return len(codes) >= 1
