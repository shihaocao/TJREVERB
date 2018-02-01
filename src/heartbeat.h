int
heartbeat()
{
  unsigned char byte_buff[BUFF_SIZE] = {0};

  int bytes_waiting = 0;
  int num_read = 0;
    while(1)
    {

      bytes_waiting = sp_input_waiting(port);
      if (bytes_waiting > 0) {
          num_read = sp_nonblocking_read(port,byte_buff, sizeof byte_buff);
          print_buffer(byte_buff,num_read);
      }
    }
}
