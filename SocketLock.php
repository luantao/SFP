<?php

/**
 * SocketLock
 *
 * @author 栾涛 <286976625@qq.com>
 */
class CacheLock {

	private $host;
	private $port;
	private $_service;
	private $connect_signal = "ok";

	public function __construct($host = "127.0.0.1", $port = "8888") {
			$this->host = $host;
			$this->port = $port;
	}

	public function checkStatus() {
		$signal = fread($this->_service, strlen($this->connect_signal));
		if ($signal == $this->connect_signal) {
			return TRUE;
		} else {
			fclose($this->_service);
			return FALSE;
		}
	}

	public function lock($key) {
		$this->_service=fsockopen($this->host, $this->port, $error, $errstr, 12);
		if(!$this->_service){
			return FALSE;
		}
		fwrite($this->_service, $key, strlen($key));
		if ($this->checkStatus()) {
			return $this->checkStatus();
		} else {
			return FALSE;
		}
	}

	public function unlock() {
		if($this->_service){
			fclose($this->_service);
			$this->_service=false;
		}
	}

}
