<?php
    include('dbcon.php');
    include('check.php');
   
	$user_id = $_SESSION['user_id'];
   
    if (is_login()){
        if ($_SESSION['user_id'] == $user_id && $_SESSION['is_car']==1) 
            ;
		else
			header("Location: index.php"); 
    }else
        header("Location: index.php"); 


    if(isset($_GET['del_carnumber']))
    {
	$stmt = $con->prepare('update users SET carnumber=NULL, is_car=0 WHERE carnumber=:del_carnumber');
	$stmt->bindParam(':del_carnumber',$_GET['del_carnumber']);
	$stmt->execute();
	
	header('refresh:0');
	sleep(1);
	header("Location:logout.php");
    }

?>
