# import argparse

# parser = argparse.ArgumentParser(description='Process some integers.')
# parser.add_argument('integers', metavar='N', type=int, nargs='+',
#                     help='an integer for the accumulator')
# parser.add_argument('--sum', dest='accumulate', action='store_const',
#                     const=sum, default=max,
#                     help='sum the integers (default: find the max)')

# args = parser.parse_args()
# print(args.accumulate(args.integers))


# Import smtplib for the actual sending function
import smtplib

# Import the email modules we'll need
from email.message import EmailMessage

msg = EmailMessage()
msg.set_content('提醒您洗澡!')

# me == the sender's email address
# you == the recipient's email address
msg['Subject'] = '提醒您每日洗澡的邮件'
msg['From'] = 'Turtle <qq769711153@hotmail.com>'
msg['To'] = '769711153@qq.com'

print(msg.as_string())


import subprocess



subprocess.run(['msmtp', '-t', '769711153@qq.com'], input=msg.as_string().encode())