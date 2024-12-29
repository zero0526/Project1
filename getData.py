import requests
def download(fileId):
    url = f"http://vrp.galgos.inf.puc-rio.br/media/com_vrp/instances/Solomon/{fileId}"
    response = requests.get(url)
    print(url)
    if response.status_code == 200:
        with open(f"./vrptwData/{fileId}", "w") as file:
            substrings = response.text.splitlines()
            result = '\n'.join(substrings[9:])
            result = substrings[4] + '\n' + result
            file.write(result)
            
        print("File đã được tải xuống thành công!")
    else:
        print(f"Đã xảy ra lỗi. Mã lỗi: {response.status_code}")
        
for i in range(1,9):
    fileId = f'RC2{i:02d}.txt'
    download(fileId)