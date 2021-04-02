def unittest(data_path, temp_path):
    import image
    img = image.Image("unittest/data/shapes.ppm", copy_to_fb=True)
    circles = img.find_circles(threshold = 5000, x_margin = 30, y_margin = 30, r_margin = 30)
    return len(circles) >= 1
