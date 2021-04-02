def unittest(data_path, temp_path):
    import image
    thresholds = [(0, 100, 56, 95, 41, 74),  # generic_red_thresholds
                  (0, 100, -128, -22, -128, 99),  # generic_green_thresholds
                  (0, 100, -128, 98, -128, -16)]     # generic_blue_thresholds
    # Load image
    img = image.Image("unittest/data/blobs.ppm", copy_to_fb=True)

    blobs = img.find_blobs(thresholds, pixels_threshold=2000, area_threshold=200)
    return  [int(x) for x in blobs[0][0:-5]] == [122, 41, 97, 82, 6228, 168, 82] and\
            [int(x) for x in blobs[1][0:-5]] == [44, 40, 78, 90, 5113, 80, 84]   and\
            [int(x) for x in blobs[2][0:-5]] == [210, 40, 72, 83, 3890, 249, 76]
