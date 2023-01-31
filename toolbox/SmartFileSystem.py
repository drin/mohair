from mohair.services import SmartFileService

if __name__ == '__main__':
    file_server = SmartFileService()
    file_server.init_storage()
    file_server.serve()
