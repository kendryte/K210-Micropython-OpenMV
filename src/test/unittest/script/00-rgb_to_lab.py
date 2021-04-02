def unittest(data_path, temp_path):
    import image
    lab = image.rgb_to_lab((120, 200, 120))
    return  (lab[0] == 74 and lab[1] == -38 and lab[2] == 30)

if __name__ == "__main__":
    TEST_DIR    = "unittest"
    TEMP_DIR    = "../temp"
    DATA_DIR    = "../data"
    SCRIPT_DIR  = "unittest/script"
    print("")
    test_failed = False

    def print_result(test, passed):
        s = "Unittest (%s)"%(test)
        padding = "."*(60-len(s))
        print(s + padding + ("PASSED" if passed == True else "FAILED"))

    test_passed = True
    try:
        gc.collect()
        if unittest(DATA_DIR, TEMP_DIR) == False:
            raise Exception()
    except Exception as e:
        test_failed = True
        test_passed = False
    print_result(test, test_passed)