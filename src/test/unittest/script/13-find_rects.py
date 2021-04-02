def unittest(data_path, temp_path):
    import image
    img = image.Image("unittest/data/qc_b1.bmp", copy_to_fb=True)
    rects = img.find_rects()
    return len(rects)>=1
