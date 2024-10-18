import cv2

def test_camera(index):
    cap = cv2.VideoCapture(index)
    if not cap.isOpened():
        print(f"Cannot open camera at index {index}")
        return
    print(f"Camera at index {index} opened successfully. Press 'q' to exit.")

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Can't receive frame. Exiting...")
            break
        cv2.imshow(f'Camera {index}', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    for idx in range(0, 38):  # Adjust the range based on your devices
        print(f"Testing camera at index {idx}")
        test_camera(idx)
